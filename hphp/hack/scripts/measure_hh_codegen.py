#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
from subprocess import PIPE, Popen, run
import sys
from tempfile import mkstemp, NamedTemporaryFile

BUCK_EXECUTABLE = 'tools/build/buck/bin/buck'
DIFF_TMP = 'mktemp /tmp/diff_hh_codegen.XXXXXX'
TIMEOUT_BIN = '/usr/bin/timeout 30s'
SCUBA_LOGGING_TARGET = '//hphp/hack/scripts:log_hh_codegen_results_to_scuba'

HACK_BUILD = "{} build @mode/dbgo //hphp/hack/src:hh_single_compile" \
    .format(BUCK_EXECUTABLE)
HACK_FIND_PATH = "{} targets @mode/dbgo --show-output \
    //hphp/hack/src:hh_single_compile".format(BUCK_EXECUTABLE)
HACK_OPTIONS = "-v Eval.EnableHipHopSyntax=1 \
    -v Hack.Compiler.OptimizeNullCheck=0 -v Hack.Compiler.OptimizeCuf=0 \
    -v Hack.Compiler.ConstantFolding=0"

HHVM_BUILD = "{} build @mode/dbgo //hphp/hhvm:hhvm".format(BUCK_EXECUTABLE)
HHVM_FIND_PATH = "{} targets @mode/dbgo --show-output //hphp/hhvm:hhvm" \
    .format(BUCK_EXECUTABLE)
HHVM_OPTIONS = "-m dumphhas -v Eval.AllowHhas=1 -v Eval.EnableHipHopSyntax=1 \
    -v Eval.DisableHphpcOpts=1 -v Eval.DisassemblerSourceMapping=0 \
    -v Eval.DisassemblerDocComments=0"

COMMON_LINES_DIFF = "/bin/diff -bBd --unchanged-group-format='%=' \
    --old-group-format='' --new-group-format='' --changed-group-format=''"

RED = '\033[031m'
GREEN = '\033[032m'
YELLOW = '\033[033m'
NO_COLOR = '\033[0m'

COLOR_OPERATOR_MAP = {'<': RED, '>': GREEN, '|': YELLOW}


def get_cli_options():
    parser = argparse.ArgumentParser(description='Hack codegen comparison')
    parser.add_argument(
        'path', metavar='<php_file_directory>',
        help="Directory of php files to test")
    args = parser.parse_args()

    return args.path


def validate_cli_options(phpPath):
    cwd = os.getcwd()
    if os.path.basename(cwd) != 'fbcode':
        print('ERROR: Wrong working directory ${}'.format(cwd))
        print('Run this script from fbcode root directory')
        sys.exit(1)
    if not os.path.isdir(phpPath):
        print('ERROR: {} is not a directory'.format(phpPath))
        sys.exit(1)


def buildHHVM():
    envWithHHVMUseBuck = os.environ.copy()
    envWithHHVMUseBuck['HHVM_USE_BUCK'] = '1'

    run(HHVM_BUILD, env=envWithHHVMUseBuck, shell=True)

    hhvmPathProc = Popen(HHVM_FIND_PATH, shell=True, stdout=PIPE)
    hhvmPathProc.wait()
    hhvmPathPipe = hhvmPathProc.communicate()[0].splitlines()[0]

    return hhvmPathPipe[hhvmPathPipe.index(b' ') + 1:].decode()


def buildHackC():
    hackProc = Popen(HACK_BUILD, shell=True)
    hackProc.wait()

    hackPathProc = Popen(HACK_FIND_PATH, shell=True, stdout=PIPE)
    hackPathProc.wait()
    # Output is of the form <buck_target> <path>
    hackLocationOutput = hackPathProc.communicate()[0].splitlines()[0]
    return hackLocationOutput[hackLocationOutput.index(b' ') + 1:].decode()


def calculateLineDiffs(
        hhvmPath, hackPath, fullPhpPath, hhvmTempFile, hackTempFile):
    run(
        '{} {} {} {}'.format(TIMEOUT_BIN, hhvmPath, HHVM_OPTIONS, fullPhpPath),
        shell=True,
        stdout=hhvmTempFile)
    run(
        '{} {} {} {}'.format(TIMEOUT_BIN, hackPath, HACK_OPTIONS, fullPhpPath),
        shell=True,
        stdout=hackTempFile)

    hhvmTempFile.seek(0)
    hackTempFile.seek(0)

    numLinesHhvm = sum(1 for line in hhvmTempFile)
    numLinesHack = sum(1 for line in hackTempFile)

    commonLinesProc = run(
        '{} {} {}'.format(
            COMMON_LINES_DIFF, hhvmTempFile.name, hackTempFile.name),
        shell=True,
        stdout=PIPE)
    numLinesCommon = sum(1 for line in commonLinesProc.stdout)

    return numLinesHhvm, numLinesHack, numLinesCommon


def writeLines(diffResultsFile, fullPhpPath, linesToWrite):
    diffLine = "***************************************************\
        ****\n"
    diffLine += "Diff details for {}\n".format(fullPhpPath)
    for line in linesToWrite:
        if len(line) < 63:
            diffLine += NO_COLOR + line.decode() + NO_COLOR + '\n'
        else:
            operator = line[62]
            colorCode = COLOR_OPERATOR_MAP.get(chr(operator), NO_COLOR)
            diffLine += colorCode + line.decode() + NO_COLOR + '\n'
    diffResultsFile.write(diffLine)


if __name__ == '__main__':
    phpPath = get_cli_options()
    validate_cli_options(phpPath)

    hhvmPath = buildHHVM()
    hackPath = buildHackC()
    phpFiles = [file for file in os.listdir(phpPath) if file.endswith('.php')]

    numLinesHhvm = 0
    numLinesHack = 0
    numLinesCommon = 0

    diffResultsFd, diffResultsFilename = mkstemp()
    with os.fdopen(diffResultsFd, 'w') as diffResultsFile:
        for phpFile in phpFiles:
            fullPhpPath = os.path.join(phpPath, phpFile)
            with NamedTemporaryFile() as hhvmTempFile, \
                    NamedTemporaryFile() as hackTempFile:
                newLinesHhvm, newLinesHack, newLinesCommon = calculateLineDiffs(
                    hhvmPath, hackPath, fullPhpPath, hhvmTempFile, hackTempFile)
                numLinesHhvm += newLinesHhvm
                numLinesHack += newLinesHack
                numLinesCommon += newLinesCommon

                diffProc = Popen(
                    '/bin/diff -bBd -y {} {} | expand'.format(
                        hhvmTempFile.name, hackTempFile.name),
                    shell=True,
                    stdout=PIPE)
                diffStdout = diffProc.communicate()[0]
                diffProc.wait()

                writeLines(
                    diffResultsFile, fullPhpPath, diffStdout.splitlines())
        run('{} run {} -- {} {} {} {} {}'.format(
            BUCK_EXECUTABLE,
            SCUBA_LOGGING_TARGET,
            numLinesHhvm,
            numLinesHack,
            numLinesCommon,
            phpPath,
            diffResultsFilename), shell=True)
        os.remove(diffResultsFilename)
