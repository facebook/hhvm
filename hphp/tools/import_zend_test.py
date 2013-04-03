#!/usr/bin/env python

"""
Copies all the Zend tests to test/zend/bad, runs them in interp mode,
then copies the good ones to test/zend/good
"""

import argparse
import os
import re
import subprocess
import sys

bad_tests = (
    '/unset_cv05.php',
    '/unset_cv06.php',
)

use_hhvm_output = (
    '/001.php',
    '/002.php',
    '/003.php',
)

errors = (
    # generic inconsistencies
    ('Variable passed to ([^\s]+)\(\) is not an array or object', 'Invalid operand type was used: expecting an array'),
)

parser = argparse.ArgumentParser()
parser.add_argument(
    "-z",
    "--zend_path",
    type=str,
    help="zend path to import tests from."
)
parser.add_argument(
    "-n",
    "--dont_run",
    action='store_true',
    help="don't run run_verify.sh. Just parse the .diff files."
)
args = parser.parse_args()


def split(pattern, str):
    return re.split(r'\n\s*--'+pattern+'--\s*\n', str, 1)

def walk(filename):
    print "Importing %s" % filename
    zend = file(filename).read()
    boom = split('FILE', zend)
    if len(boom) != 2:
        print "Malformed test, no --FILE--: ", filename
        return

    name, therest = boom

    boom = split('EXPECT', therest)
    if len(boom) == 2:
        test, exp = boom
    else:
        boom = split('EXPECTF', therest)
        if len(boom) == 2:
            test, exp = boom
        else:
            print "Malformed test, no --EXPECT-- or --EXPECTF--: ", filename
            return

    dest_filename = os.path.basename(filename).replace('.phpt', '.php')
    cur_dir = os.path.dirname(__file__)
    dest_dir = os.path.join(cur_dir, '../test/zend/bad')
    full_dest_filename = os.path.join(dest_dir, dest_filename)

    if 'bug60771.php' in full_dest_filename:
        test = test.replace("?>", "unlink('test.php');\n?>")

    exp = exp.replace('in %s on', 'in %s/%s on' % ('hphp/test/zend/bad', dest_filename))

    # PHP puts a newline in that we don't
    exp = exp.replace('\n\nFatal error:', '\nFatal error:')
    exp = exp.replace('\n\nWarning:', '\nWarning:')
    exp = exp.replace('\n\nNotice:', '\nNotice:')

    exp = exp.replace('Fatal error:', 'HipHop Fatal error:')
    exp = exp.replace('Warning:', 'HipHop Warning:')
    exp = exp.replace('Notice:', 'HipHop Notice:')

    for error in errors:
        exp = re.sub(error[0], error[1], exp)

    file(full_dest_filename, 'w').write(test)
    file(full_dest_filename+'.exp', 'w').write(exp)

if args.zend_path:
    path = os.path.join(args.zend_path, 'Zend/tests/')
    for root, dirs, files in os.walk(path):
        for filename in files:
            if not '.phpt' in filename:
                continue
            walk(os.path.join(root, filename))

if not args.dont_run:
    env = os.environ
    env.update({'VQ':'interp', 'TEST_PATH':'zend/bad'})
    proc = subprocess.Popen(['tools/run_verify.sh'], env=env)
    proc.wait()

def isOkDiff(original_name):
    for test in bad_tests:
        if test in original_name:
            return False
    for test in use_hhvm_output:
        if test in original_name:
            return True

    filename = original_name + '.diff'
    # no diff file or is empty
    if not os.path.exists(filename) or os.stat(filename)[6] == 0:
        return True

    lines = file(filename).read()
    # line 123 -> line %d
    lines = re.sub(r'line \d+\n', 'line %d\n', lines)
    # kill identical lines
    lines = re.sub(r'-(.*)\n\+\1\n', '', lines, re.DOTALL)

    # kill header
    lines = re.sub(r'---.*', '', lines)
    lines = re.sub(r'\+\+\+.*', '', lines)

    lines = lines.split('\n')
    matches = []
    for line in lines:
        line = line.strip()
        if not line:
            continue
        if line[0] == '-':
            line = '+' + line[1:]
            line = line.replace('%unicode|string%', 'string')
            matches.append(line)
        elif line[0] == '+':
            if not matches:
                return False
            match = matches.pop(0)

            # emulate run-tests.php I guess...
            i,j = 0,0
            mode = None
            while j < len(line) and i < len(match):
                if match[i] == '%':
                    mode = match[i+1]
                    i += 2
                elif mode is None:
                    if match[i] != line[j]:
                        return False
                    i += 1
                elif mode == 's':
                    if line[j] == ' ':
                        mode = None
                elif mode == 'd':
                    if not line[j].isdigit():
                        mode = None
                elif mode == 'a':
                    pass
                else:
                    raise Exception('what is %' + mode + '\n' + line)
                j += 1
            
            if i < len(match) - 1:
                return False

    return True

for root, dirs, files in os.walk('test/zend/bad'):
    for filename in files:
        if not filename.endswith('.php'):
            continue
        filename = os.path.join(root, filename)
        good_file = filename.replace('bad', 'good', 1)
        if isOkDiff(filename):
            os.rename(filename, good_file)
            file(good_file+'.exp', 'w').write(
                file(filename+'.out').read().replace('/bad', '/good')
            )
            os.unlink(filename+'.exp')

        else:
            if not os.path.exists(good_file):
                continue
            os.unlink(good_file)
            os.unlink(good_file+'.exp')
