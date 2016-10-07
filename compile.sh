#!/bin/bash
basedir=$(cd `dirname $0`; pwd)
verbose=0
runOnSuccess=0
debugOnSuccess=0
mainFile=""
readFiles=""
includedFiles=""
sourceFiles=""

declare -a iDirectories
declare -a sDirectories
declare -a objects

function debug ()
{
	if [ ${verbose} -eq 1 ]
	then
		echo "[o] "$*
	fi
}

function log ()
{
	echo "[+] "$*
}

function error ()
{
	echo "[x] "$* 1>&2
}

function usage ()
{
	error "usage: $(basename $0) <file>"
	exit 1
}

function getIncludedFiles ()
{
	if [ $(echo -e "${readFiles}" | grep "$1" | wc -l) = "0" ]
	then		
		includes=$(grep -o "^#include\s*<[^>]\+" $* 2>/dev/null | sed 's/^[^<]\+<//g')
	
		files=""
		for file in $*
		do
			if [ -z "${file}" ]
			then
				files="${file}"
			else
				files="${files}\n${file}"
			fi
		done
	
		if [ -z "${readFiles}" ]
		then
			readFiles="${files}"
		else
			readFiles="${readFiles}\n${files}"
		fi
	
		OLDIFS=${IFS}
		IFS=$'\n'
		for include in ${includes}
		do
			IFS=${OLDIFS}
			
			for iDirectory in ${iDirectories[*]}
			do
				if [ -e "${iDirectory}/${include}" ]
				then
					if [ $(echo -n "${includedFiles}" | grep "${include}" | wc -l) = "0" ]
					then
						for sDirectory in ${sDirectories[*]}
						do
							sourcefile=""
							
							if [ -e "${sDirectory}/${include%.h}.cpp" ]
							then
								sourcefile="${sDirectory}/${include%.h}.cpp"
								
							elif [ -e "${sDirectory}/${include%.h}.c" ]
							then
								sourcefile="${sDirectory}/${include%.h}.c"
							fi
							
							if [ ! -z "${sourcefile}" ]
							then
								if [ -z "${includedFiles}" ]
								then
									includedFiles="${include}"
									sourceFiles="${sourcefile}"
								else
									includedFiles="${includedFiles}\n${include}"
									sourceFiles="${sourceFiles}\n${sourcefile}"
								fi

								getIncludedFiles ${iDirectory}/${include} ${sourcefile}
								
								break
							fi
						done
					fi
					
					break
				fi
			done
			
			IFS=$'\n'
		done
		IFS=${OLDIFS}
	fi
}

function compile ()
{
	source="$1"
	library="$2"
	
	if [ -e "${source}" ]
	then
		objects[${#objects[*]}]="${library}"
	
		if [ ! -e "${library}" -o "${source}" -nt "${library}" ]
		then
			libraryType=""
			filename=""
			
			if [ "${source:0:${#externalSourceDirectory}}" = "${externalSourceDirectory}" ]
			then
				libraryType="Library"
				filename="${source:$(expr ${#externalSourceDirectory} + 1)}"
	
			elif [ "${source:0:${#sourceDirectory}}" = "${sourceDirectory}" ]
			then
				libraryType="Project"
				filename="${source:$(expr ${#sourceDirectory} + 1)}"
			else
				libraryType="Project"
				filename="${source}"
			fi
				
			log "Compiling [${libraryType}] \"${filename}\"..."
			mkdir -p $(dirname ${library})
			
			# Option -x c for C files (with g++) should not be handled in this script
			cOptions=""
			
			if [[ ${source} == *.c ]]
			then
				cOptions=" -x c"
			fi
			
			debug "${compiler}${cOptions} ${compilerOptions} ${flags} ${source} -o ${library}"
			${compiler}${cOptions} ${compilerOptions} ${flags} ${source} -o ${library}
			
			if [ "$?" != "0" ]
			then
				error "Compilation error. Exiting."
				exit 6
			fi
		else
			debug "Skipping \"${source}\"."
		fi
	else
		error "Source file \"${source}\" does not exists."
		exit 5
	fi
}

function getBinaryName ()
{
	echo "${binaryDirectory}/`basename $(echo "${mainFile}" | sed 's/\.c\(pp\)\?$//')`${binarySuffix}"
}

function link ()
{
	log "Linking objects..."
	executable="$(getBinaryName)"
	mkdir -p $(dirname ${executable})
	
	debug "${linker} -o ${executable} ${objects[*]} ${linkerOptions}"
	${linker} -o ${executable} ${objects[*]} ${linkerOptions}
	
	if [ "$?" != "0" ]
	then
		error "Linking error. Exiting."
		exit 7
	fi
}

function run ()
{
	prefix=""
	
	if [ "${debugOnSuccess}" = "1" ]
	then
		prefix="${debugger} "
	fi
	
	if [ "${runOnSuccess}" = "1" ]
	then
		${prefix} $(getBinaryName)
	fi
}

# Initialization
if [ ! -e "${basedir}/targets/base.conf" ]
then
	error "File \"${basedir}/targets/base.conf\" does not exists. Exiting."
	exit 2
fi

source "${basedir}/targets/base.conf"
target=""
targetConfiguration="${defaultTarget}"

# Parsing options
justPrintExecutableName=0

while getopts ":v:t:r:d:e:" opt; do
	case "${opt}" in
		t)
			targetConfiguration=${OPTARG}
			;;
		v)
			verbose=1
			OPTIND=$(expr ${OPTIND} - 1)
			;;
		r)
			runOnSuccess=1
			OPTIND=$(expr ${OPTIND} - 1)
			;;
		d)
			runOnSuccess=1
			debugOnSuccess=1
			OPTIND=$(expr ${OPTIND} - 1)
			;;
		e)
			justPrintExecutableName=1
			OPTIND=$(expr ${OPTIND} - 1)
			;;
		*)
			usage
			;;
	esac
done
shift $((OPTIND-1))

mainFile=$1

if [ -z "${mainFile}" ]
then
	usage
fi

if [ ! -e "${mainFile}" ]
then
	error "Specified file \"${mainFile}\" does not exists. Exiting."
	exit 3
fi

if [ ! -e "${basedir}/targets/${targetConfiguration}.conf" ]
then
	error "No configuration file for target \"${targetConfiguration}\" (${basedir}/targets/${targetConfiguration}.conf). Exiting."
	exit 4
fi

source "${basedir}/targets/${targetConfiguration}.conf"

libraryDirectory="${libraryDirectory}/${target}"
binarySuffix=".${target}${binarySuffix}"

if [ "${justPrintExecutableName}" = "1" ]
then
	echo "$(getBinaryName)"
	exit 0
fi

log "Target: ${target}"
log "Main File: ${mainFile}"
log "Binary: " $(echo `getBinaryName` | sed "s|^${basedir}/||")
log "-----"

if [ "$(which git &>/dev/null ; echo $?)" = "0" ]
then
	projectBranch="$(git status 2>&1 | head -n1 | sed 's|^On branch ||g' | sed '/^fatal:/d')"
	libraryBranch="$(cd ${externalSourceDirectory} ; git status 2>&1 | head -n1 | sed 's|^On branch ||g' | sed '/^fatal:/d')"
	
	if [ -z "${projectBranch}" ]
	then
		projectBranch="<Not a git repository>"
	fi
	
	if [ -z "${libraryBranch}" ]
	then
		libraryBranch="<Not a git repository>"
	fi
	
	log "Project branch: ${projectBranch}"
	log "Library branch: ${libraryBranch}"
	log "-----"
fi

startTimestamp=$(date +%s)

# Parse file
iDirectories[0]="${externalIncludeDirectory}"
iDirectories[1]="${includeDirectory}"
sDirectories[0]="${externalSourceDirectory}"
sDirectories[1]="${sourceDirectory}"

log "Looking for dependencies..."
getIncludedFiles ${mainFile}
log "$(echo -e ${sourceFiles} | wc -l) dependencies found."

# Compilation
compile ${mainFile} ${libraryDirectory}/$(echo ${mainFile} | sed 's/\.c\(pp\)\?$/.o/')

OLDIFS=${IFS}
IFS=$'\n'
for file in $(echo -e "${sourceFiles}")
do
	IFS=${OLDIFS}
	
	library=""
	
	if [ "${file:0:${#externalSourceDirectory}}" = "${externalSourceDirectory}" ]
	then
		library="${libraryDirectory}${file:${#externalSourceDirectory}}"
	
	elif [ "${file:0:${#sourceDirectory}}" = "${sourceDirectory}" ]
	then
		library="${libraryDirectory}${file:${#sourceDirectory}}"
	fi
	
	library=$(echo ${library} | sed 's/\.c\(pp\)\?$/.o/')
	
	compile ${file} ${library}
	
	IFS=$'\n'
done
IFS=${OLDIFS}

# Link
link

log "-----"
log "Compilation successful."

endTimestamp=$(date +%s)

elapsedHours=0
elapsedMinutes=0
elapsedSeconds=$(expr ${endTimestamp} - ${startTimestamp})
elapsedTime=""

while [ ${elapsedSeconds} -ge 60 ]
do
	elapsedMinutes=$(expr ${elapsedMinutes} + 1)
	elapsedSeconds=$(expr ${elapsedSeconds} - 60)
done

while [ ${elapsedMinutes} -ge 60 ]
do
	elapsedHours=$(expr ${elapsedHours} + 1)
	elapsedMinutes=$(expr ${elapsedMinutes} - 60)
done

if [ ${elapsedHours} -gt 0 ]
then
	elapsedTime="${elapsedTime}${elapsedHours} h "
fi

if [ ${elapsedHours} -gt 0 -o ${elapsedMinutes} -gt 0 ]
then
	elapsedTime="${elapsedTime}${elapsedMinutes} mn "
fi

elapsedTime="${elapsedTime}${elapsedSeconds} s"

log "Elapsed time: ${elapsedTime}"

# Run (if asked for)
run
