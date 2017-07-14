#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import argparse
import os
import re
import shutil
import sys
import tarfile
import urllib2

# The version that we will be importing the tests from.
# Must be a valid, released version from php download site
zend_version = "7.1.6"

no_import = ()

# only import these tests
whitelist = (
    "Zend/tests",
    "tests/lang",
)

other_files = (
    "tests/quicktester.inc",
)


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError:  # Python >2.5
        pass


def walk(filename, dest_subdir):
    dest_filename = os.path.basename(filename)

    mkdir_p(dest_subdir)
    full_dest_filename = os.path.join(dest_subdir, dest_filename)

    # Exactly mirror zend's directories incase some tests depend on random crap.
    # We'll only move things we want into 'good'. The condition below is in case
    # of using --local on a file already in the current workind directory.
    if (os.path.abspath(filename) != os.path.abspath(full_dest_filename) and
            not full_dest_filename.endswith(".phpt")):
        shutil.copyfile(filename, full_dest_filename)

    full_dest_filename = full_dest_filename.replace('.phpt', '.php')

    if '.phpt' not in filename:
        if full_dest_filename.endswith('.php'):
            with open(full_dest_filename.replace('.php', '.php.skipif'), 'w') as f:
                f.write('always skip - not a test')

        return

    print("Importing %s" % full_dest_filename)

    def split(pattern, str):
        return re.split(r'\n\s*--' + pattern + '--\s*\n', str, 1)

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

    sections = parse_headers(open(filename).read())
    for i in list(sections.keys()):
        sections[i] = '\n'.join(sections[i])

    unsupported_sections = ('POST_RAW')
    for name in unsupported_sections:
        if name in sections:
            print("Unsupported test with section --%s--: " % name, filename)
            return

    if 'FILE' not in sections:
        print("Malformed test, no --FILE--: ", filename)
        return

    for key in ('EXPECT', 'EXPECTF', 'EXPECTREGEX'):
        if key in sections:
            exp = sections[key]

            # tests are really inconsistent about whitespace
            exp = re.sub(r'(\r\n|\r|\n)', '\n', exp.strip())

            # PHP puts a newline in that we don't
            exp = exp.replace('\nParse error:', 'Parse error:')
            exp = exp.replace('\nFatal error:', 'Fatal error:')
            exp = exp.replace('\nCatchable fatal error:', 'Catchable fatal error:')
            exp = exp.replace('\nWarning:', 'Warning:')
            exp = exp.replace('\nNotice:', 'Notice:')

            match_rest_of_line = '%s'
            if key == 'EXPECTREGEX':
                match_rest_of_line = '.+'

            exp = re.sub(r'(?:Parse|Fatal)\\? error\\?:.*',
                         '\nFatal error: ' + match_rest_of_line, exp)
            exp = re.sub(r'(?:Catchable fatal)\\? error\\?:.*',
                         '\nCatchable fatal error: ' + match_rest_of_line, exp)
            exp = re.sub(r'Warning\\?:.*',
                         '\nWarning: ' + match_rest_of_line, exp)
            exp = re.sub(r'Notice\\?:.*',
                         '\nNotice: ' + match_rest_of_line, exp)
            exp = re.sub(r'object\((\w+)\)#\d+', 'object(\\1)#%d', exp)

            exp = re.sub('string\(7\) "Closure"', 'string(%d) "Closure%s"', exp)
            exp = re.sub('Closure(?!%s)', 'Closure%s', exp)

            sections[key] = exp

    if 'EXPECT' in sections:
        exp = sections['EXPECT']

        # we use %a for error messages so always write expectf
        open(full_dest_filename + '.expectf', 'w').write(exp)
    elif 'EXPECTREGEX' in sections:
        exp = sections['EXPECTREGEX']

        open(full_dest_filename + '.expectregex', 'w').write(exp)
    elif 'EXPECTF' in sections:
        exp = sections['EXPECTF']

        open(full_dest_filename + '.expectf', 'w').write(exp)
    else:
        print("Malformed test, no --EXPECT*--: ", filename)
        return

    if 'INI' in sections:
        ini = sections['INI']

        open(full_dest_filename + '.ini', 'w').write(ini)

    if 'SKIPIF' in sections:
        skipif = sections['SKIPIF'].strip()
        if skipif[:2] != '<?':
            skipif = '<?php' + skipif

        open(full_dest_filename + '.skipif', 'w').write(skipif)

    test = sections['FILE'] + "\n"

    if 'POST' in sections:
        test = test.replace(
            '<?php',
            '<?php\nparse_str("' + sections['POST'] + '", $_POST);\n'
            '$_REQUEST = array_merge($_REQUEST, $_POST);\n'
            '_filter_snapshot_globals();\n'
        )
    if 'GET' in sections:
        test = test.replace(
            '<?php',
            '<?php\nparse_str("' + sections['GET'] + '", $_GET);\n'
            '$_REQUEST = array_merge($_REQUEST, $_GET);\n'
            '_filter_snapshot_globals();\n'
        )
    if 'COOKIE' in sections:
        test = test.replace(
            '<?php',
            ('<?php\n$_COOKIE = http_parse_cookie("' +
                 sections['COOKIE'] + '");\n' +
                 '_filter_snapshot_globals();\n')
        )
    if 'ENV' in sections:
        for line in sections['ENV'].split('\n'):
            boom = line.split('=')
            if len(boom) == 2 and boom[0] and boom[1]:
                test = test.replace(
                    '<?php',
                    '<?php\n$_ENV[%s] = %s;\n'
                    '_filter_snapshot_globals();\n'
                    % (boom[0], boom[1])
                )

    if 'CLEAN' in sections:
        if not re.search(r'<\?php.*\?>', test, re.DOTALL):
            test += '?>\n'
        if not test.endswith('\n'):
            test += '\n'
        if not sections['CLEAN'].startswith('<?'):
            sections['CLEAN'] = '<?php\n' + sections['CLEAN']
        disable_errors = "<?php error_reporting(0); ?>\n"
        test += disable_errors + sections['CLEAN']

    if not full_dest_filename.endswith('.phpt'):
        open(full_dest_filename, 'w').write(test)


def should_import(filename):
    for bad in no_import:
        if bad in filename:
            return False
    return True


# -- main --

parser = argparse.ArgumentParser()
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
parser.add_argument(
    "--local",
    type=str,
    help="Convert a single local .phpt into *.php and expect files in the CWD.")
args = parser.parse_args()

if args.local:
    if args.only:
        print("--only and --local are not compatible; choose one.")
        sys.exit(1)
    if not os.path.exists(args.local):
        print("--local file not found")
        sys.exit(1)
    if not args.local.endswith('.phpt'):
        print("Invalid --local file. Must be a .phpt file")
        sys.exit(1)

    print("Converting %s into php and expect files..." % (args.local, ))
    walk(args.local, os.getcwd())

    sys.exit(0)

script_dir = os.path.dirname(__file__)
zend_dir = os.path.normpath(os.path.join(script_dir, '../zend7'))
all_dir = os.path.join(zend_dir, 'all')

zend_release_name = "php-" + zend_version
zend_release_filename = zend_release_name + ".tar.gz"
zend_release_archive = os.path.join(zend_dir, zend_release_filename)
zend_release_path = os.path.join(zend_dir, zend_release_name)

if not os.path.isfile(zend_release_archive):
    print('Downloading ' + zend_release_name + '...')
    zend_release_url = ("http://php.net/get/" +
                        zend_release_filename +
                        "/from/this/mirror")
    zend_release_request = urllib2.Request(zend_release_url)
    zend_release_response = urllib2.urlopen(zend_release_request)
    open(zend_release_archive, 'wb').write(zend_release_response.read())

if not os.path.isdir(zend_release_path):
    print('Extracting ' + zend_release_name + '...')
    zend_release_tar = tarfile.open(zend_release_archive, 'r:gz')
    zend_release_tar.extractall(zend_dir)

for root, _dirs, files in os.walk(zend_release_path):
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

        if matches(whitelist) and should_import(full_file):
            walk(
                full_file,
                os.path.join(zend_dir, os.path.relpath(root, zend_release_path))
            )

if not args.dirty:
    shutil.rmtree(zend_release_path)
