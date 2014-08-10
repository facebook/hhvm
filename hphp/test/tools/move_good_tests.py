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

def moveAllFiles(old, new):
    files = [
        '',  # original file
        '.expect',
        '.expectf',
        '.expectregex',
        '.opts',
        '.noserver',
        '.norepo',
        '.ini'
    ]
    for filesuffix in files:
        if os.path.isfile(old + filesuffix):
            shutil.move(old + filesuffix, new + filesuffix)

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

run = subprocess.Popen(
  [
      os.path.join(script_dir, "../run"),
      '--fbmake',
      args.folder
  ],
  stdout=subprocess.PIPE,
  stderr=subprocess.STDOUT
).communicate()[0]

last_line = run.strip().split("\n")[-1]
results = json.loads(last_line)['results']


good_tests = getGoodTests(results)

if args.verbose or args.no_move:
    print "\nGood tests:"
    for test in good_tests:
        print test.replace(os.path.realpath(zend_dir), '')

if not args.no_move:
    print "\nMoving tests"
    moveTests(good_tests)
