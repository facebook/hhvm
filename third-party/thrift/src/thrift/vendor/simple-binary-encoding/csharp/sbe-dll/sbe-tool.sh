#! /usr/bin/env bash

ROOTDIR=$(dirname ${0})
SBE_JAR=${ROOTDIR}/sbe-tool-all.jar

[ -f "${SBE_JAR}" ] || (echo "Missing ${SBE_JAR}"; exit 1)

function usage {
    echo usage: $(basename "${0}")  [-d output_dir] -s schema
}

# defaults
OUTPUTDIR=.

while getopts "d:s:" OPT "$@"; do
    case ${OPT} in
    d)
        OUTPUTDIR=${OPTARG}
        ;;
    s)
        SCHEMA=${OPTARG}
        ;;
    *)
        echo "${OPT}"
        usage
        exit 1
        ;;
    esac
done
shift $((${OPTIND} - 1))

# Check args
if [ -z "${SCHEMA}" ]; then usage; exit 1; fi
if [ ! -f "${SCHEMA}" ]; then echo no schema at "${SCHEMA}"; exit 1; fi

java \
-Dsbe.output.dir="${OUTPUTDIR}" \
-Dsbe.generate.ir="false" \
-Dsbe.target.language="uk.co.real_logic.sbe.generation.csharp.CSharp" \
-jar "${SBE_JAR}" \
"${SCHEMA}"
