#!/usr/bin/env python

# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2
# @lint-avoid-python-3-compatibility-imports

#
# Copies all the phplang-spec tests to a temporary directory, runs them in
# interp mode, then copies the good ones to test/spec/good and the bad ones to
# test/spec/bad.
#

import argparse
import glob
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import urllib2
import zipfile

# Don't even pull these into the repo.
# We want running the bad tests to still complete.
no_import = (
    '.gitignore',
    'README.md',
    'make_phpt',
)

# For marking tests as always failing. Used to keep flaky tests in flaky/.
flaky_tests = ()

# Tests that work but not in repo mode
norepo_tests = (
    # TODO: See if any of these should work in repo mode
    '/classes/setting_state.php',
    '/expressions/error_control_operator/error_control.php',
    '/expressions/general/order_of_evaluation.php',
    '/expressions/primary_expressions/intrinsics_eval.php',
    '/tests/expressions/primary_expressions/intrinsics_unset.php',
    '/expressions/source_file_inclusion/include.php',
    '/expressions/source_file_inclusion/require.php',
    '/functions/basics.php',
    '/functions/byrefs.php',
)

# Random other files that zend wants
other_files = (
    '/classes/Aircraft.inc',
    '/classes/MathLibrary.inc',
    '/classes/MyCollection.inc',
    '/classes/PassengerJet.inc',
    '/classes/Point.inc',
    '/classes/Point2.inc',
    '/classes/Vehicle.inc',
    '/constants/includefile.inc',
    '/exception_handling/MyRangeException.inc',
    '/expressions/primary_expressions/Point.inc',
    '/expressions/primary_expressions/Point2.inc',
    '/expressions/source_file_inclusion/Circle.inc',
    '/expressions/source_file_inclusion/Point.inc',
    '/expressions/source_file_inclusion/Positions.inc',
    '/expressions/yield_operator/Testfile.txt',
    '/functions/TestInc.inc',
    '/interfaces/MyCollection.inc',
    '/interfaces/MyList.inc',
    '/namespaces/Circle.inc',
    '/namespaces/Point.inc',
    '/serialization/Point.inc',
    '/config.ini',
)

parser = argparse.ArgumentParser()
parser.add_argument(
    "-o",
    "--only",
    type=str,
    action='append',
    help="only import tests whose path contains this substring."
)
parser.add_argument(
    "--dirty",
    action='store_true',
    help="leave around test/spec/all directory."
)
parser.add_argument(
    "-v",
    "--verbose",
    action='store_true',
    help="print out extra stuff."
)
args = parser.parse_args()


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        pass

def walk(filename, dest_subdir):
    dest_filename = os.path.basename(filename)

    mkdir_p(dest_subdir)
    full_dest_filename = os.path.join(dest_subdir, dest_filename)

    # Exactly mirror spec's directories
    # We'll only move things we want into 'good'
    shutil.copyfile(filename, full_dest_filename)

    print("Importing %s" % full_dest_filename)

def should_import(filename):
    for bad in no_import:
        if bad in filename:
            return False

    if 'phpt' in os.path.dirname(os.path.realpath(filename)):
        return False

    if 'tests' in os.path.dirname(os.path.realpath(filename)):
        return True

    return False

def index_containing_substring(the_list, substring):
    if not the_list:
        return -1
    for i, s in enumerate(the_list):
        if substring in s:
              return i
    return -1

script_dir = os.path.dirname(__file__)
spec_dir = os.path.normpath(os.path.join(script_dir, '../spec'))
all_dir = os.path.join(spec_dir, 'all')

spec_release_name = "php-langspec-master"
spec_release_filename = spec_release_name + ".zip"
spec_release_archive = os.path.join(spec_dir, spec_release_filename)
spec_release_path = os.path.join(spec_dir, spec_release_name)

# Any tests that fail will be added to the global skipif via a makework PHP
# program by this script. It gets emptied out every time the
# script is run
skip_code_file = spec_dir + '/config.skipif'
skip_code_file_bak = spec_dir + '/config.skipif.bak'
# make a backup in case something goes wrong and we pass dirty
shutil.copyfile(skip_code_file, skip_code_file_bak)
prev_skip_content = open(skip_code_file,'r').read().splitlines()
# empty the skipif file so we can run all the tests again without
# skipping them
open(skip_code_file, 'w').write('')

if not os.path.exists(spec_dir):
    os.makedirs(spec_dir)

if not os.path.isfile(spec_release_archive):
    print('Downloading ' + spec_release_name + '...')
    spec_release_url = 'https://github.com/JoelMarcey/'
    spec_release_url = spec_release_url + 'php-langspec/archive/master.zip'
    spec_release_request = urllib2.urlopen(spec_release_url)
    output = open(spec_release_archive, "w")
    output.write(spec_release_request.read())
    output.close()

if not os.path.isdir(spec_release_path):
    print('Extracting ' + spec_release_name + '...')
    with zipfile.ZipFile(spec_release_archive, 'r') as zf:
        zf.extractall(spec_dir)

for root, dirs, files in os.walk(spec_release_path):
    for filename in files:
        full_file = os.path.join(root, filename)

        def matches(patterns):
            if not patterns:
                return True
            for other_file in other_files:
                if not other_file.endswith('.phpt') and other_file in full_file:
                    return True
            for pattern in patterns:
                if pattern in full_file:
                    return True
            return False

        if (
                matches(args.only) and
                should_import(full_file)
           ):
            walk(
                full_file,
                os.path.join(all_dir, os.path.relpath(root, spec_release_path))
            )

if not os.path.isdir(all_dir):
    if args.only:
        print("No test/spec/all. " +
              "Your --only arg didn't match any test that should be imported.")
    else:
        print("No test/spec/all. Maybe no tests were imported?")
    sys.exit(0)
else:
    print("Running all tests from spec/all")

stdout = subprocess.Popen(
    [
        os.path.join(script_dir, '../run'),
        '--fbmake',
        '-m',
        'interp',
        all_dir,
    ],
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT
).communicate()[0]

# segfaults also print on stderr
stdout = re.sub('\nsh: line 1:.*', '', stdout)
# just the last line
last_line = stdout.strip().split("\n")[-1]
results = json.loads(last_line)['results']

if args.verbose:
    print(results)
else:
    print("Test run done, moving files")

skip_code = '<?php' + os.linesep + os.linesep + '$need_fix = array('
skip_code = skip_code + os.linesep
for test in results:
    filename = test['name']

    dest_file = filename.replace('all', '.', 1)
    mkdir_p(os.path.dirname(dest_file))

    good = (test['status'] == 'passed')

    flaky_test = False
    for test in flaky_tests:
        if test in filename:
            good = False
            flaky_test = True

    needs_norepo = False
    for test in norepo_tests:
        if test in filename:
            needs_norepo = True

    if not good:
        idx = index_containing_substring(prev_skip_content, dest_file)
        if idx != -1:
            skip_code = skip_code + prev_skip_content[idx]
        else:
            skip_code = skip_code + '  "' + dest_file
            skip_code = skip_code + '", // ADD WHY COMMENT HERE'
        skip_code = skip_code + os.linesep

    exps = glob.glob(filename + '.expect*')
    if not exps:
        # this file is probably generated while running tests :(
        continue

    source_file_exp = exps[0]
    _, dest_ext = os.path.splitext(source_file_exp)
    shutil.copyfile(filename, dest_file)
    open(dest_file + dest_ext, 'w').write(
        open(source_file_exp).read().replace('/all', '/.')
    )

    if needs_norepo:
        open(dest_file + '.norepo', 'w')
    if os.path.exists(filename + '.skipif'):
        shutil.copyfile(filename + '.skipif', dest_file + '.skipif')
    if os.path.exists(filename + '.ini'):
        shutil.copyfile(filename + '.ini', dest_file + '.ini')

skip_code = skip_code + ');' + os.linesep
skip_code = skip_code + '$test = "./" '
skip_code = skip_code + '. substr($argv[1], strrpos($argv[1], "tests/"));'
skip_code = skip_code + os.linesep + 'if (in_array($test, $need_fix)) {'
skip_code = skip_code + os.linesep + '  echo "skip";' + os.linesep + '}'
skip_code = skip_code + os.linesep
open(skip_code_file, 'w').write(skip_code)

# extra random files needed for tests...
for root, dirs, files in os.walk(all_dir):
    for filename in files:
        filename = os.path.join(root, filename)

        for name in other_files:
            if name in filename:
                # root config.ini file goes in directory level above
                # the tests themselves
                if 'config.ini' in filename:
                    dest = spec_dir + '/config.ini'
                    data = open(filename).read()
                    open(dest, 'w').write(data)
                else:
                    dest = filename.replace('all', '.', 1)
                    dir = os.path.dirname(dest)
                    mkdir_p(dir)
                    data = open(filename).read()
                    open(dest, 'w').write(data)

if not args.dirty:
    shutil.rmtree(all_dir)
    shutil.rmtree(spec_release_path)
    os.remove(skip_code_file_bak)
