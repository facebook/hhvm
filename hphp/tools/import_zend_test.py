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
    'phar', # this appears in filenames

    # works in interp but not others
    'bug25922.php',
    'bug34064.php',
)

errors = (
    # generic inconsistencies
    ('Variable passed to ([^\s]+)\(\) is not an array or object', 'Invalid operand type was used: expecting an array'),
    ('bcdiv\(\): ', ''),
    ('bcsqrt\(\): ', ''),
    ('bcpowmod\(\): ', ''),

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
    
    dest_filename = os.path.basename(filename).replace('.phpt', '.php')
    source_dir = source.lower().replace('/tests', '').replace('/', '-')

    for key in ('EXPECT', 'EXPECTF', 'EXPECTREGEX'):
        if sections.has_key(key):
            exp = sections[key]
            
            # tests are really inconsistent about whitespace
            exp = re.sub(r'(\r\n|\r|\n)', '\n', exp.strip())

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

            sections[key] = exp
    
    if sections.has_key('EXPECT'):
        exp = sections['EXPECT']
    elif sections.has_key('EXPECTF'):
        wanted_re = sections['EXPECTF']

        # from run-tests.php
        temp = "";
        r = "%r";
        startOffset = 0;
        length = len(wanted_re);
        while startOffset < length:
            start = wanted_re.find(r, startOffset)
            if start != -1:
                end = wanted_re.find(r, start+2);
                if end == -1:
                    # unbalanced tag, ignore it.
                    end = start = length;
            else:
                start = end = length;
        
            temp = temp + re.escape(wanted_re[startOffset:start - startOffset])
            if (end > start):
                temp = temp + '(' + wanted_re[start+2:end - start-2] + ')'
            
            startOffset = end + 2

        wanted_re = temp

        ## different from php, since python escapes %
        wanted_re = wanted_re.replace('\\%', '%')

        wanted_re = wanted_re.replace('%binary_string_optional%', 'string')
        wanted_re = wanted_re.replace('%unicode_string_optional%', 'string')
        wanted_re = wanted_re.replace('%unicode\|string%', 'string')
        wanted_re = wanted_re.replace('%string\|unicode%', 'string')
        wanted_re = wanted_re.replace('%u\|b%', '')
        wanted_re = wanted_re.replace('%b\|u%', '')
        
        # Stick to basics
        wanted_re = wanted_re.replace('%e', '\\/')
        wanted_re = wanted_re.replace('%s', '[^\r\n]+')
        wanted_re = wanted_re.replace('%S', '[^\r\n]*')
        wanted_re = wanted_re.replace('%a', '.+')
        wanted_re = wanted_re.replace('%A', '.*')
        wanted_re = wanted_re.replace('%w', '\s*')
        wanted_re = wanted_re.replace('%i', '[+-]?\d+')
        wanted_re = wanted_re.replace('%d', '\d+')
        wanted_re = wanted_re.replace('%x', '[0-9a-fA-F]+')
        wanted_re = wanted_re.replace('%f', '[+-]?\.?\d+\.?\d*(?:[Ee][+-]?\d+)?')
        wanted_re = wanted_re.replace('%c', '.')
        exp = wanted_re

    elif sections.has_key('EXPECTREGEX'):
        exp = sections['EXPECTREGEX']
    else:
        print "Malformed test, no --EXPECT-- or --EXPECTF-- or --EXPECTREGEX--: ", filename
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

    cur_dir = os.path.dirname(__file__)
    dest_subdir = os.path.join(cur_dir, '../test/zend/all', source_dir)
    mkdir_p(dest_subdir)
    full_dest_filename = os.path.join(dest_subdir, dest_filename)

    if 'bug60771.php' in full_dest_filename:
        test = test.replace("?>", "unlink('test.php');\n?>")
    if 'bug44805.php' in full_dest_filename:
        test = test.replace("1)) {\n\tunlink($file2", "2)) {\n\tunlink($file2")

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

            # not implemented extensions
            '/sapi',
            '/ext/calendar',
            '/ext/com_dotnet',
            '/ext/dba',
            '/ext/dom',
            '/ext/enchant',
            '/ext/ereg',
            '/ext/fileinfo',
            '/ext/filter',
            '/ext/ftp',
            '/ext/gett/ext',
            '/ext/gmp',
            '/ext/interbase',
            '/ext/mssql',
            '/ext/mysqli',
            '/ext/mysqlnd',
            '/ext/opcache',
            '/ext/odbc',
            '/ext/pdo_dblib',
            '/ext/pdo_firebird',
            '/ext/pdo_odbc',
            '/ext/pdo_pgsql',
            '/ext/pdo_oci',
            '/ext/phar',
            '/ext/pspell',
            '/ext/readline',
            '/ext/recode',
            '/ext/reflection',
            '/ext/shmop',
            '/ext/snmp',
            '/ext/sybase_ct',
            '/ext/sysvmsg',
            '/ext/sysvsem',
            '/ext/sysvshm',
            '/ext/tidy',
            '/ext/tokenizer',
            '/ext/wddx',
            '/ext/xmlrpc',
            '/ext/xsl',
            '/ext/zip',
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
                if should_import(full_file):
                    walk(full_file, root.replace(args.zend_path, ''))

if not os.path.isdir('test/zend/all'):
    print "No test/zend/all. Maybe no tests were imported?"
    sys.exit(0)

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
            global args
            for test in bad_tests:
                if test in original_name:
                    return False

            # no diff file or is empty
            if (not os.path.exists(original_name + '.diff') or \
                os.stat(original_name + '.diff')[6] == 0):
                if args.verbose:
                    print '\n', original_name, '\nNo .diff, passed'
                return True

            # PHP is very inconsistent with whitespace in tests
            diff = file(original_name + '.diff').read()
            diff = re.sub(r'-(.*)\n\\ No newline at end of file\n\+\1', '', diff)
            if not re.search(r'\n-', diff):
                if args.verbose:
                    print '\n', original_name, '\nOnly whitespace .diff, passed'
                return True

            # I hack a bit and store the regex in the .exp file, use that
            wanted_re = file(original_name + '.exp').read().strip()
            output = file(original_name + '.out').read().strip()
            
            import sre_constants
            try:
                match = re.match(wanted_re, output)
            except (OverflowError, AssertionError, sre_constants.error) as e:
                if args.verbose:
                    print '\n', original_name, '\nException', '\n', e
                return False

            if args.verbose:
                print '\n', original_name, '\n', repr(wanted_re), '\n', repr(output), '\n', match is not None
            return match and match.group() == output


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
