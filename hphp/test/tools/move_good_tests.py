#!/usr/bin/env python

"""
Searches for good tests in zend/bad and moves them to good
"""

# @lint-avoid-python-3-compatibility-imports

import argparse
import json
import os
import subprocess
import shutil

def getGoodTests(tests):
    good = []
    for test in tests:
        if test['status'] == "passed":
            good.append(test['name'])
    return good

def runGoodTestsRepo(tests):
    results = getResults("-r", tests)
    return getGoodTests(results)

def getResults(flags, folder):
    run = subprocess.Popen(
        [
            os.path.join(script_dir, "../run"),
            '--fbmake',
            flags
        ] + folder,  # It needs every file separate, not a string
                     # else ./run barfs
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT
    ).communicate()[0]
    last_line = run.strip().split("\n")[-1]
    return json.loads(last_line)['results']

def moveAllFiles(old, new):
    files = [
        '',  # original file
        '.expect',
        '.expectf',
        '.expectregex',
        '.opts',
        '.noserver',
        '.norepo',
        '.ini',
        '.skipif'
    ]
    for filesuffix in files:
        if os.path.isfile(old + filesuffix):
            subprocess.call(["git", "mv", old + filesuffix, new + filesuffix])

def moveTests(tests):
    for test in tests:
        new_test = test.replace('bad', 'good', 1)
        moveAllFiles(test, new_test)
        if args.verbose:
            old_file = test.replace(os.path.realpath(zend_dir), '')
            new_file = new_test.replace(os.path.realpath(zend_dir), '')
            print old_file, '=>', new_file

script_dir = os.path.dirname(__file__)
zend_dir = os.path.join(script_dir, "../zend/")

parser = argparse.ArgumentParser(
    "Searches good tests in zend/bad and moves them to good"
)

parser.add_argument(
    "--folder",
    "-f",
    default=os.path.join(zend_dir, './bad'),
    help="The folder to search in. Default: zend/bad"
)

parser.add_argument(
    "--verbose",
    "-v",
    action="store_true",
    help="Be chatty."
)

parser.add_argument(
    "--no-move",
    help="Don't move the tests. Only output them.",
    action="store_true"
)

args = parser.parse_args()

print "Searching in ", os.path.realpath(args.folder)


results = getResults("", [args.folder])
good_tests = getGoodTests(results)
good_tests = runGoodTestsRepo(good_tests)


if args.verbose or args.no_move:
    print "\nGood tests:"
    for test in good_tests:
        print test.replace(os.path.realpath(zend_dir), '')

if not args.no_move:
    if len(good_tests) == 0:
        print "\nNo good tests found"
    else:
        print "\nMoving %d tests" % len(good_tests)
        moveTests(good_tests)
