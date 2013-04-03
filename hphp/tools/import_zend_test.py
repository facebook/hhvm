#!/usr/bin/env python

"""
Copies all the Zend tests to a temporary directory, runs them in interp mode,
then copies the good ones to test/zend/good and the bad ones to test/zend/bad.
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
    'bug29971.php',
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

    # works in interp but not others
    'bug25922.php',
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
parser.add_argument(
    "--dirty",
    action='store_true',
    help="leave around test/zend/all directory."
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
    for i in sections.keys():
        sections[i] = '\n'.join(sections[i])

    if not sections.has_key('FILE'):
        print "Malformed test, no --FILE--: ", filename
        return

    test = sections['FILE']
    
    if sections.has_key('EXPECT'):
        exp = sections['EXPECT']
    elif sections.has_key('EXPECTF'):
        exp = sections['EXPECTF']
    else:
        print "Malformed test, no --EXPECT-- or --EXPECTF--: ", filename
        return

    if sections.has_key('POST'):
        test = test.replace(
            '<?php', 
            '<?php\nparse_str("' + sections['POST'] + '", $_POST);\n'
        )
    if sections.has_key('GET'):
        test = test.replace(
            '<?php', 
            '<?php\nparse_str("' + sections['GET'] + '", $_GET);\n'
        )
    if sections.has_key('COOKIE'):
        test = test.replace(
            '<?php', 
            '<?php\n$_COOKIE = http_parse_cookie("' + sections['COOKIE'] + '");\n'
        )

    unsupported_sections = ('INI', 'POST_RAW')
    for name in unsupported_sections:
        if sections.has_key(name):
            print "Unsupported test with section --%s--: " % name, filename
            return

    dest_filename = os.path.basename(filename).replace('.phpt', '.php')
    cur_dir = os.path.dirname(__file__)
    source_dir = source.lower().replace('/tests', '').replace('/', '-')
    dest_subdir = os.path.join(cur_dir, '../test/zend/all', source_dir)
    mkdir_p(dest_subdir)
    full_dest_filename = os.path.join(dest_subdir, dest_filename)

    if 'bug60771.php' in full_dest_filename:
        test = test.replace("?>", "unlink('test.php');\n?>")
    if 'bug44805.php' in full_dest_filename:
        test = test.replace("1)) {\n\tunlink($file2", "2)) {\n\tunlink($file2")

    exp = exp.replace('in %s on', 'in %s/%s/%s on' % ('hphp/test/zend/all', source_dir, dest_filename))

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
    env.update({'VQ':'interp', 'TEST_PATH':'zend/all'})
    proc = subprocess.Popen(['tools/run_verify.sh'], env=env)
    proc.wait()

for root, dirs, files in os.walk('test/zend/all'):
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

        good_file = filename.replace('all', 'good', 1)
        bad_file = filename.replace('all', 'bad', 1)
        mkdir_p(os.path.dirname(good_file))
        mkdir_p(os.path.dirname(bad_file))

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
            dest_file = good_file
            source_file_exp = filename+'.out'
            delete_file = bad_file
            subpath = 'good'
        else:
            dest_file = bad_file
            source_file_exp = filename+'.exp'
            delete_file = good_file
            subpath = 'bad'

        os.rename(filename, dest_file)
        file(dest_file+'.exp', 'w').write(
            file(source_file_exp).read().replace('/all', '/' + subpath)
        )
        if os.path.exists(delete_file):
            os.unlink(delete_file)
        if os.path.exists(delete_file+'.exp'):
            os.unlink(delete_file+'.exp')

if not args.dirty:
    import shutil
    shutil.rmtree('test/zend/all')
