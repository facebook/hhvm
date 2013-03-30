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

    if '%s' in exp:
        exp = exp.replace('in %s on', 'in %s/%s on' % ('hphp/test/zend/bad', dest_filename))

    # PHP puts a newline in that we don't
    exp = exp.replace('\n\nFatal error:', '\nFatal error:')
    exp = exp.replace('\n\nWarning:', '\nWarning:')
    exp = exp.replace('\n\nNotice:', '\nNotice:')

    exp = exp.replace('Fatal error:', 'HipHop Fatal error:')
    exp = exp.replace('Warning:', 'HipHop Warning:')
    exp = exp.replace('Notice:', 'HipHop Notice:')

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

def isOkDiff(original_name):
    filename = original_name + '.diff'
    # no diff file or is empty
    if not os.path.exists(filename) or os.stat(filename)[6] == 0:
        return True

    lines = file(filename).read()
    # kill header
    lines = re.sub(r'---.*', '', lines)
    lines = re.sub(r'\+\+\+.*', '', lines)
    # line 123 -> line %d
    lines = re.sub(r'line \d+\n', 'line %d\n', lines)
    # kill identical lines
    lines = re.sub(r'-(.*)\n\+\1\n', '', lines)
    
    return re.search(r'\n(\+|\-)', lines) is None

for filename in set(test_files):
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
