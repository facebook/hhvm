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
    # SESSION is bused on husdon
    'unset_cv05.php', 
    'unset_cv06.php',

    # unpredictable numbers - we need param matching
    'bug35143.php',
    'gettimeofday_basic.php',
    'localtime_basic.php',
    'time_basic.php',
    'posix_getpgid_basic.php',
    'posix_getpgid_basic.php',
    'posix_getpgrp_basic.php',
    'posix_getpid_basic.php',
    'posix_getppid_basic.php',
    'posix_getsid_basic.php',
    'posix_times_basic.php',
    'socket_getsockname.php',
    'fileinode_variation1.php',
    'filestat.php',
    'fstat_basic.php',
    'fstat.php',
    'touch_basic.php',
    'openssl_random_pseudo_bytes.php',
    'pcntl_fork_basic.php',
    'posix_times.php',
    'array_diff_assoc_variation6.php',
    'bug39322.php',
    'getmypid_basic.php',
    'getrusage_basic.php',
   
    # not implemented extensions
    'phar',
)

errors = (
    # generic inconsistencies
    ('Variable passed to ([^\s]+)\(\) is not an array or object', 'Invalid operand type was used: expecting an array'),

    # I can't do math with backreferences so write them out
    ('([^\s]+)\(\) expects exactly 1 parameter, 0 given', r'Missing argument 1 for \1()'),
    ('([^\s]+)\(\) expects exactly (\d+) parameters, \d+ given', r'Missing argument \2 for \1()'),
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
parser.add_argument(
    "-o",
    "--only",
    type=str,
    help="only import tests whose path matches this regex."
)
args = parser.parse_args()


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        pass

def walk(filename, source):
    print "Importing %s" % filename

    def split(pattern, str):
        return re.split(r'\n\s*--'+pattern+'--\s*\n', str, 1)

    def parse_headers(zend):
        sections = {}
        cur_header = None
        for line in zend.split('\n'):
            header = re.match('--([_A-Z]+)--', line)
            if header:
                cur_header = header.group(1)
                sections[cur_header] = []
            else:
                sections[cur_header].append(line)
        return sections

    sections = parse_headers(file(filename).read())
    if not sections.has_key('FILE'):
        print "Malformed test, no --FILE--: ", filename
        return

    test = '\n'.join(sections['FILE'])
    
    if sections.has_key('EXPECT'):
        exp = '\n'.join(sections['EXPECT'])
    elif sections.has_key('EXPECTF'):
        exp = '\n'.join(sections['EXPECTF'])
    else:
        print "Malformed test, no --EXPECT-- or --EXPECTF--: ", filename
        return

    dest_filename = os.path.basename(filename).replace('.phpt', '.php')
    cur_dir = os.path.dirname(__file__)
    source_dir = source.lower().replace('/tests', '').replace('/', '-')
    dest_dir = os.path.join(cur_dir, '../test/zend/bad', source_dir)
    mkdir_p(dest_dir)
    full_dest_filename = os.path.join(dest_dir, dest_filename)

    if 'bug60771.php' in full_dest_filename:
        test = test.replace("?>", "unlink('test.php');\n?>")
    if 'bug44805.php' in full_dest_filename:
        test = test.replace("1)) {\n\tunlink($file2", "2)) {\n\tunlink($file2")

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
    test_dirs = (('Zend/tests'), ('tests'), ('sapi'), ('ext'))
    def should_import(filename):
        no_import = (
            # these hang forever
            '005a.php',
            'array_012.php',
            'array_pad_variation2.php',
            'bug27508.php',
            'gzgetc_basic.php',
            'gzgets_basic.php',
            'observer_003.php',
            'observer_004.php',
            'observer_005.php',
            'observer_006.php',
            'observer_009.php',
            'sleep_error.php',
            'socket_select-wrongparams-1.php',
            'test010.php',
            'usleep_error.php',
        )
        if not '.phpt' in filename:
            return False
        for bad in no_import:
            if bad in filename:
                return False
        return True

    for source in test_dirs:
        for root, dirs, files in os.walk(os.path.join(args.zend_path, source)):
            for filename in files:
                full_file = os.path.join(root, filename)
                if args.only and not re.search(args.only, full_file):
                    continue
                if should_import(filename):
                    walk(full_file, root.replace(args.zend_path, ''))

if not args.dont_run:
    env = os.environ
    env.update({'VQ':'interp', 'TEST_PATH':'zend/bad'})
    proc = subprocess.Popen(['tools/run_verify.sh'], env=env)
    proc.wait()

for root, dirs, files in os.walk('test/zend/bad'):
    for filename in files:
        if not filename.endswith('.php'):
            continue
        filename = os.path.join(root, filename)
        
        def all_exist(filename):
            extensions = ('', '.out', '.exp')
            for ext in extensions:
                if not os.path.exists(filename+ext):
                    # something crazy is going on
                    return False
            return True
        if not all_exist(filename):
            continue

        good_file = filename.replace('bad', 'good', 1)
        mkdir_p(os.path.dirname(good_file))

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
            lines = lines.replace('\n\ No newline at end of file', '')

            lines = lines.split('\n')
            matches = []
            for line in lines:
                line = line.strip()
                if not line:
                    continue
                if line[0] == '-':
                    line = '+' + line[1:]
                    line = line.replace('%unicode|string%', 'string')
                    line = line.replace('%u|b%', '')
                    line = line.replace('%b|u%', '')
                    line = line.replace('%e', '/')
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
                        elif mode == 'd' or mode == 'i' or mode == 'r':
                            if not line[j].isdigit():
                                mode = None
                        elif mode == 'f':
                            if not line[j].isdigit() and line[j] != '.':
                                mode = None
                        elif mode == 'a': # anything
                            pass
                        elif mode == 'c': # one character
                            mode = None
                        elif mode == '\0': # literal percent
                            if line[j] != '%':
                                return False
                            mode = None
                        elif mode == 'w': # not quite sure...
                            mode = None
                        elif mode == 'C' or mode == 'A': # again, not sure
                            mode = None
                        else:
                            raise Exception('what is "%' + mode + '"\n' + match + '\n' + original_name)
                        j += 1
                    
                    if i < len(match) - 1:
                        return False

            return True

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
