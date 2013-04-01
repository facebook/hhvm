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

bad_tests = (
    'unset_cv05.php', 
    'unset_cv06.php',
)

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
if not os.environ.has_key('NO_TEST'):
    proc = subprocess.Popen(['tools/run_verify.sh'], env=env)
    proc.wait()

def isOkDiff(original_name):
    for test in bad_tests:
        if test in original_name:
            return False

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
