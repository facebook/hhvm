#!/usr/bin/env python

"""
Copies all the Zend tests to test/zend/bad, runs them in interp mode,
then copies the good ones to test/zend/good
"""

import os
import re
import subprocess
import sys

if len(sys.argv) == 1:
    print "Usage: \n\n  %s /tmp/php-5.4.11/" % sys.argv[0]
    sys.exit(0)

test_files = []

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

    test_files.append(full_dest_filename)

    if 'in %s on' in exp:
        exp = exp.replace('in %s on', 'in %s/%s on' % ('hphp/test/zend/bad', dest_filename))
    exp = exp.replace('Fatal error:', 'HipHop Fatal error:')
    exp = exp.replace('Warning:', 'HipHop Warning:')
    exp = exp.replace('Notice:', 'HipHop Notice:')
    # you'll have to fix up the line %d yourself

    file(full_dest_filename, 'w').write(test)
    file(full_dest_filename+'.exp', 'w').write(exp)

path = os.path.join(sys.argv[1], 'Zend/tests/')
for root, dirs, files in os.walk(path):
    for filename in files:
        if not '.phpt' in filename:
            continue
        walk(os.path.join(root, filename))

env = os.environ
env.update({'VQ':'interp', 'TEST_PATH':'zend/bad'})
proc = subprocess.Popen(['tools/run_verify.sh'], env=env)
proc.wait()

def isOkDiff(file):
    if not os.path.exists(file+'.diff') or os.stat(file+'.diff')[6] == 0:
        return True
    return False

for file in test_files:
    for ext in ['', '.exp']:
        goodFile = file.replace('bad', 'good', 1)
        if isOkDiff(file):
            if os.path.exists(file+ext):
                os.rename(file+ext, goodFile+ext)
        else:
            if os.path.exists(goodFile+ext):
                os.unlink(goodFile+ext)
