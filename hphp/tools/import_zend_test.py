#!/usr/bin/env python

"""
Copies all the Zend tests to a temporary directory, runs them in interp mode,
then copies the good ones to test/zend/good and the bad ones to test/zend/bad.
"""

import argparse
import glob
import json
import os
import re
import shutil
import subprocess
import sys

# Don't even pull these into the repo.
# We want runnig the bad tests to still complete.
no_import = (
    # these hang forever
    '/ext/sockets/tests/socket_select-wrongparams-1.phpt',
    '/ext/spl/tests/array_012.phpt',
    '/ext/spl/tests/observer_003.phpt',
    '/ext/spl/tests/observer_004.phpt',
    '/ext/spl/tests/observer_005.phpt',
    '/ext/spl/tests/observer_006.phpt',
    '/ext/spl/tests/observer_009.phpt',
    '/ext/standard/tests/array/array_pad_variation2.phpt',
    '/ext/standard/tests/file/bug27508.phpt',
    '/ext/standard/tests/general_functions/sleep_error.phpt',
    '/ext/standard/tests/general_functions/usleep_error.phpt',
    '/ext/zlib/tests/gzgetc_basic.phpt',
    '/ext/zlib/tests/gzgets_basic.phpt',
    '/tests/func/005a.phpt',
    '/tests/run-test/test010.phpt',

    # segfaults
    '/Zend/tests/bug35239.phpt',
    '/Zend/tests/bug54265.phpt',
    '/Zend/tests/bug55705.phpt',
    '/Zend/tests/callable_type_hint_001.phpt',
    '/Zend/tests/callable_type_hint_003.phpt',
    '/Zend/tests/heredoc_005.phpt',
    '/Zend/tests/jump13.phpt',
    '/ext/bz2/tests/004.phpt',
    '/ext/date/tests/bug50055.phpt',
    '/ext/gd/tests/crafted_gd2.phpt',
    '/ext/pcntl/tests/pcntl_exec.phpt',
    '/ext/pcntl/tests/pcntl_exec_2.phpt',
    '/ext/pcntl/tests/pcntl_exec_3.phpt',
    '/ext/session/tests/bug61728.phpt',
    '/ext/session/tests/session_module_name_variation2.phpt',
    '/tests/func/010.phpt',
    '/tests/lang/bug21820.phpt',
    '/tests/lang/func_get_arg.003.phpt',
    '/tests/lang/func_get_args.003.phpt',
    '/tests/lang/func_num_args.003.phpt',
    '/tests/lang/operators/divide_basiclong_64bit.phpt',
    '/tests/lang/operators/modulus_basiclong_64bit.phpt',

    # intermittent segfaults
    '/Zend/tests/001.phpt',
    '/Zend/tests/002.phpt',
    '/Zend/tests/003.phpt',

    # spews files until they work
    '/ext/spl/tests/SplFileInfo_getExtension_basic.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_basic.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_delimiter_basic.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_delimiter_error.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_enclosure_basic.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_enclosure_error.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_escape_basic.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_escape_default.phpt',
    '/ext/spl/tests/SplFileObject_fgetcsv_escape_error.phpt',
    '/ext/spl/tests/SplFileObject_getflags_basic.phpt',
    '/ext/spl/tests/SplFileObject_getflags_error001.phpt',
    '/ext/spl/tests/SplFileObject_getflags_error002.phpt',
    '/ext/spl/tests/SplFileObject_rewind_error001.phpt',
    '/ext/spl/tests/SplFileObject_setCsvControl_basic.phpt',
    '/ext/spl/tests/SplFileObject_setCsvControl_error001.phpt',
    '/ext/spl/tests/SplFileObject_setCsvControl_error002.phpt',
    '/ext/spl/tests/SplFileObject_setCsvControl_variation001.phpt',
    '/ext/standard/tests/file/fopen_variation14-win32.phpt',
    '/ext/standard/tests/file/fopen_variation15-win32.phpt',
    '/ext/standard/tests/file/mkdir_variation5-win32.phpt',
    '/ext/standard/tests/file/mkdir_variation5-win32.phpt',
    '/ext/standard/tests/file/readfile_variation4.phpt',
    '/ext/standard/tests/file/windows_links/bug48746.phpt',
    '/ext/standard/tests/file/windows_links/bug48746_1.phpt',
    '/ext/standard/tests/file/windows_links/bug48746_2.phpt',
    '/ext/xmlwriter/tests/001.phpt',
    '/ext/xmlwriter/tests/004.phpt',
    '/ext/xmlwriter/tests/005.phpt',
    '/ext/xmlwriter/tests/006.phpt',
    '/ext/xmlwriter/tests/OO_001.phpt',
    '/ext/xmlwriter/tests/OO_004.phpt',
    '/ext/xmlwriter/tests/OO_005.phpt',
    '/ext/xmlwriter/tests/OO_006.phpt',

    # not tests
    '/ext/xmlwriter/examples/',

    # not implemented extensions
    '/ext/calendar',
    '/ext/com_dotnet',
    '/ext/dba',
    '/ext/dom',
    '/ext/enchant',
    '/ext/ereg',
    '/ext/fileinfo',
    '/ext/filter',
    '/ext/ftp',
    '/ext/gettext',
    '/ext/gmp',
    '/ext/interbase',
    '/ext/mssql',
    '/ext/mysqli',
    '/ext/mysqlnd',
    '/ext/oci8',
    '/ext/odbc',
    '/ext/pdo_dblib',
    '/ext/pdo_firebird',
    '/ext/pdo_oci',
    '/ext/pdo_odbc',
    '/ext/pdo_pgsql',
    '/ext/phar',
    '/ext/pspell',
    '/ext/readline',
    '/ext/recode',
    '/ext/reflection',
    '/ext/shmop',
    '/ext/skeleton',
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
    '/sapi',
)

# For mark these as failing
bad_tests = (
    # SESSION is bused on husdon
    '/zend/unset_cv05.php',
    '/zend/unset_cv06.php',

    # not implemented extensions
    'phar', # this appears in filenames

    # works in interp but not others
    '/tests-lang/bug25922.php',
    '/zend/bug34064.php',
    '/zend/objects_029.php',
    '/zend/objects_030.php',
    '/ext-standard-strings/bug44242.php',
    '/ext-sqlite3/sqlite3_29_createfunction.php',

    # line number is inconsistent on stack overflow
    '/zend/bug41633_3.php',

    # broken in Jenkins
    '/ext-standard-array/array_diff_assoc_variation6.php',
    '/zend/bug35239.php',
    '/zend/bug54265.php',

    # broken in contbuild
    '/ext-standard-strings/setlocale_variation1.php',
    '/ext-standard-strings/setlocale_basic1.php',
    '/ext-standard-strings/setlocale_basic2.php',
    '/ext-standard-strings/setlocale_basic3.php',
    '/ext-standard-file/filetype_variation2.php',
    '/ext-standard-file/filetype_variation3.php',
    '/ext-sockets/ipv4loop.php',
    '/ext-posix/posix_kill_basic.php',
    '/ext-standard-file/005_variation-win32.php',
    '/ext-json/fail001.php',

    # our build machines have no members in group 0...
    '/ext-posix/posix_getgrgid.php',
    '/ext-posix/posix_getgroups_basic.php',

    # concurrency issues
    '/ext-mysql/001.php',
    '/ext-mysql/bug47438.php',
    '/ext-mysql/mysql_client_encoding.php',
    '/ext-mysql/mysql_select_db.php',
    '/ext-sqlite3/sqlite3_08_udf.php',
    '/ext-sqlite3/sqlite3_25_create_aggregate.php',
    '/ext-sockets/socket_connect_params.php',
    '/ext-sockets/socket_getpeername_ipv4loop.php',

    # time tests are hard to write, these are poorly written
    '/ext-date/bug48187.php',

    # broken on unbuntu 12.04
    '/ext-date/DateTimeZone_listAbbreviations_basic1.php',
    '/ext-date/bug52290.php',
    '/ext-date/timezone_abbreviations_list_basic1.php',
    '/ext-gd/bug24155.php',
    '/ext-gd/bug39366.php',
    '/ext-gd/bug48732.php',
    '/ext-mbstring/bug54494.php',
    '/ext-openssl/openssl_x509_parse_basic.php',
    '/ext-standard-streams/bug61115-2.php',
    '/ext-standard-strings/moneyformat.php',
)

# Random other files that zend wants
other_files = (
    '/ext-curl/curl_testdata1.txt',
    '/ext-curl/curl_testdata2.txt',
    '/ext-exif/bug48378.jpeg',
    '/ext-gd/Tuffy.ttf',
    '/ext-gd/bug37346.gif',
    '/ext-gd/bug38112.gif',
    '/ext-gd/bug43121.gif',
    '/ext-gd/conv_test.gif',
    '/ext-gd/conv_test.jpeg',
    '/ext-gd/conv_test.png',
    '/ext-gd/conv_test.xbm',
    '/ext-gd/php.gif',
    '/ext-gd/src.gd2',
    '/ext-gd/src.wbmp',
    '/ext-gd/test8859.ttf',
    '/ext-gd/test_gif.gif',
    '/ext-intl/ut_common.inc',
    '/ext-ldap/connect.inc',
    '/ext-mbstring/common.inc',
    '/ext-mcrypt/vectors.txt',
    '/ext-mysql/connect.inc',
    '/ext-openssl/005_crt.txt',
    '/ext-openssl/bug37820cert.pem',
    '/ext-openssl/bug37820key.pem',
    '/ext-openssl/bug39217cert1.txt',
    '/ext-openssl/bug39217cert2.txt',
    '/ext-openssl/cert.crt',
    '/ext-openssl/openssl.cnf',
    '/ext-openssl/private.key',
    '/ext-openssl/public.key',
    '/ext-pdo/pdo_test.inc',
    '/ext-pdo_mysql/config.inc',
    '/ext-pdo_mysql/common.phpt',
    '/ext-pdo_sqlite/config.inc',
    '/ext-pdo_sqlite/common.phpt',
    '/ext-session/save_handler.inc',
    '/ext-simplexml/bug24392.xml',
    '/ext-soap-bugs/bug30928.wsdl',
    '/ext-soap-bugs/bug41337.wsdl',
    '/ext-soap-bugs/bug42692.wsdl',
    '/ext-soap-schema/test_schema.inc',
    '/ext-soap-soap12/soap12-test.inc',
    '/ext-soap-soap12/soap12-test.wsdl',
    '/ext-spl/testclass.class.inc',
    '/ext-sqlite3/new_db.inc',
    '/ext-sqlite3/stream_test.inc',
    '/ext-standard-general_functions/004.data',
    '/ext-standard-array/compare_function.inc',
    '/ext-standard-class_object/AutoLoaded.inc',
    '/ext-standard-class_object/AutoInterface.inc',
    '/ext-standard-class_object/AutoTrait.inc',
    '/ext-standard-file/test.csv',
    '/ext-standard-file/test2.csv',
    '/ext-standard-file/test3.csv',
    '/ext-standard-file/fopen_include_path.inc',
    '/ext-standard-image/bug13213.jpg',
    '/ext-standard-image/246x247.png',
    '/ext-standard-image/384x385.png',
    '/ext-standard-image/testAPP.jpg',
    '/ext-standard-image/test13pix.swf',
    '/ext-standard-image/test1pix.jpg',
    '/ext-standard-serialize/autoload_implements.p5c',
    '/ext-standard-serialize/autoload_interface.p5c',
    '/ext-xml/xmltest.xml',
    '/ext-xmlreader/012.dtd',
    '/ext-xmlreader/012.xml',
    '/ext-xmlreader/dtdexample.dtd',
    '/ext-xmlreader/relaxNG.rng',
    '/ext-xmlreader/relaxNG2.rng',
    '/ext-xmlreader/relaxNG3.rng',
    '/ext-zlib/004.txt.gz',
    '/tests-classes/autoload_derived.p5c',
    '/tests-classes/autoload_implements.p5c',
    '/tests-classes/autoload_interface.p5c',
    '/tests-classes/autoload_root.p5c',
    '/tests-classes/constants_basic_003.inc',
    '/tests-classes/interface_optional_arg_003.inc',
    '/tests-lang/015.inc',
    '/tests-lang/016.inc',
    '/tests-lang/023-2.inc',
    '/tests-lang/inc.inc',
    '/tests-lang/inc_throw.inc',
    '/tests/quicktester.inc',
    '/zend/bug54804.inc',
    '/zend/nowdoc.inc',
    '/zend/ns_022.inc',
    '/zend/ns_027.inc',
    '/zend/ns_028.inc',
    '/zend/ns_065.inc',
    '/zend/ns_066.inc',
    '/zend/ns_067.inc',
    '/zend/ns_069.inc',
    '/zend/unset.inc',
)

# Map strings from one style to another
errors = (
    # generic inconsistencies
    ('Variable passed to ([^\s]+)\(\) is not an array or object',
        'Invalid operand type was used: expecting an array'),
    ('bcdiv\(\): ', ''),
    ('bcsqrt\(\): ', ''),
    ('bcpowmod\(\): ', ''),
)

parser = argparse.ArgumentParser()
parser.add_argument(
    "-z",
    "--zend_path",
    type=str,
    help="zend path to import tests from."
)
parser.add_argument(
    "-o",
    "--only",
    type=str,
    action='append',
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
    dest_filename = os.path.basename(filename)
    source_dir = source.lower().replace('/tests', '').replace('/', '-')

    cur_dir = os.path.dirname(__file__)
    dest_subdir = os.path.join(cur_dir, '../test/zend/all', source_dir)
    mkdir_p(dest_subdir)
    full_dest_filename = os.path.join(dest_subdir, dest_filename)

    # Exactly mirror zend's directories incase some tests depend on random crap.
    # We'll only move things we want into 'good'
    shutil.copyfile(filename, full_dest_filename)

    full_dest_filename = full_dest_filename.replace('.phpt', '.php')

    if not '.phpt' in filename:
        return

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

    unsupported_sections = ('INI', 'POST_RAW')
    for name in unsupported_sections:
        if sections.has_key(name):
            print "Unsupported test with section --%s--: " % name, filename
            return

    if not sections.has_key('FILE'):
        print "Malformed test, no --FILE--: ", filename
        return

    for key in ('EXPECT', 'EXPECTF', 'EXPECTREGEX'):
        if sections.has_key(key):
            exp = sections[key]

            # tests are really inconsistent about whitespace
            exp = re.sub(r'(\r\n|\r|\n)', '\n', exp.strip())

            exp = exp.replace('in %s on', 'in %s/%s/%s on' %
                    ('hphp/test/zend/all', source_dir, dest_filename))

            # PHP puts a newline in that we don't
            exp = exp.replace('\n\nFatal error:', '\nFatal error:')
            exp = exp.replace('\n\nCatchable fatal error:', '\nCatchable fatal error:')
            exp = exp.replace('\n\nWarning:', '\nWarning:')
            exp = exp.replace('\n\nNotice:', '\nNotice:')

            match_rest_of_line = '%a'
            if key == 'EXPECTREGEX':
                match_rest_of_line = '.+'

            exp = re.sub(r'Fatal\\? error\\?:.*',
                    'HipHop Fatal error: '+match_rest_of_line, exp)
            exp = re.sub(r'Catchable\\? fatal\\? error\\?:.*',
                    'HipHop Fatal error: '+match_rest_of_line, exp)
            exp = re.sub(r'Warning\\?:.*',
                    'HipHop Warning: '+match_rest_of_line, exp)
            exp = re.sub(r'Notice\\?:.*',
                    'HipHop Notice: '+match_rest_of_line, exp)

            for error in errors:
                exp = re.sub(error[0], error[1], exp)

            sections[key] = exp

    if sections.has_key('EXPECT'):
        exp = sections['EXPECT']
        # we use %a for error messages so always write expectf
        file(full_dest_filename+'.expectf', 'w').write(exp)
    elif sections.has_key('EXPECTREGEX'):
        exp = sections['EXPECTREGEX']
        file(full_dest_filename+'.expectregex', 'w').write(exp)
    elif sections.has_key('EXPECTF'):
        exp = sections['EXPECTF']

        if '/ext-standard-file/tempnam_variation5.php' in full_dest_filename:
            exp = exp.replace('tempnam_variation6', 'tempnam_variation5')

        file(full_dest_filename+'.expectf', 'w').write(exp)
    else:
        print "Malformed test, no --EXPECT*--: ", filename
        return

    test = sections['FILE']

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
            '<?php\n$_COOKIE = http_parse_cookie("' +
                sections['COOKIE'] + '");\n'
        )
    if sections.has_key('ENV'):
        for line in sections['ENV'].split('\n'):
            boom = line.split('=')
            if len(boom) == 2 and boom[0] and boom[1]:
                test = test.replace(
                    '<?php',
                    '<?php\n$_ENV[%s] = %s;\n' % (boom[0], boom[1])
                )
    if sections.has_key('CLEAN'):
        test += sections['CLEAN']

    # If you put an exception in here, please send a pull request upstream to 
    # php-src. Then when it gets merged kill your hack.
    if 'bug60771.php' in full_dest_filename:
        test = test.replace("?>", "unlink('test.php');\n?>")
    if 'bug44805.php' in full_dest_filename:
        test = test.replace("1)) {\n\tunlink($file2", "2)) {\n\tunlink($file2")
    if 'bug24054.php' in full_dest_filename:
        test = test.replace("quicktester.inc", "tests/quicktester.inc")
    if 'ext-bz2/with_strings.php' in full_dest_filename:
        test = test.replace("../..", "")
    if 'ext-bz2/bug51997.php' in full_dest_filename:
        test = test.replace("testfile.bz2", "bug51997.bz2")
    if 'ext-bz2/with_files.php' in full_dest_filename:
        test = test.replace("testfile.bz2", "with_files.bz2")
    if 'ext-xmlreader/003.php' in full_dest_filename:
        test = test.replace("_002.xml", "_003.xml")
    if 'ext-xmlreader/004.php' in full_dest_filename:
        test = test.replace("_002.xml", "_004.xml")
    if 'ext-standard-file/bug45181.php' in full_dest_filename:
        test = test.replace('rmdir("bug45181_x");',
                'chdir("..");\nrmdir("bug45181_x");')
    if 'bug61139.php' in full_dest_filename:
        test += "\nunlink('someFile');\n?>"
    if '/ext-pdo_' in full_dest_filename:
        test = test.replace('/../../../ext/pdo/tests/pdo_test.inc',
                '/../ext-pdo/pdo_test.inc')
    if '/ext-zlib/gzseek_variation7.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation7.gz')
    if '/ext-zlib/gzseek_basic2.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_basic2.gz')
    if '/ext-zlib/gzseek_variation1.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation1.gz')
    if '/ext-zlib/gzseek_variation4.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation4.gz')
    if '/ext-zlib/gzseek_variation5.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation5.gz')
    if '/ext-standard-strings/vfprintf_' in full_dest_filename:
        test = test.replace('dump.txt', dest_filename + '.txt')
    if '/ext-standard-file/touch_basic.php' in full_dest_filename:
        test = test.replace('touch.dat', 'touch_basic.dat')
    if '/ext-standard-file/touch_variation2.php' in full_dest_filename:
        test = test.replace('touch.dat', 'touch_variation2.dat')
    if '/ext-standard-file/file_put_contents_variation9.php' in full_dest_filename:
        test = test.replace('fileGetContentsVar9', 'filePutContentsVar9')
    if '/ext-standard-network/tcp4loop.php' in full_dest_filename:
        test = test.replace('31337', '31338')
    if '/ext-standard-file/fpassthru_variation.php' in full_dest_filename:
        test = test.replace('passthru.tmp', 'fpassthru_variation.tmp')
    if '/ext-spl/SplFileInfo_getPerms_basic.php' in full_dest_filename:
        test = test.replace('test_file_ptfi', 'SplFileInfo_getPerms_basic.txt')
    if '/ext-spl/SplFileInfo_getInode_basic.php' in full_dest_filename:
        test = test.replace('test_file_ptfi', 'SplFileInfo_getInode_basic.txt')
    if '/ext-standard-file/mkdir-001.php' in full_dest_filename:
        test = test.replace('testdir', 'mkdir-001')
    if '/ext-standard-file/mkdir-002.php' in full_dest_filename:
        test = test.replace('testdir', 'mkdir-002')
    if '/ext-standard-file/mkdir-003.php' in full_dest_filename:
        test = test.replace('testdir', 'mkdir-003')
    if '/ext-standard-file/filesize_variation3-win32.php' in full_dest_filename:
        test = test.replace('filesize_variation3', 'filesize_variation3-win32')
    if '/ext-hash/hash_file_basic.php' in full_dest_filename:
        test = test.replace('hash_file_example.txt', 'hash_file_basic_example.txt')
    if '/ext-hash/hash_file_error.php' in full_dest_filename:
        test = test.replace('hash_file_example.txt', 'hash_file_error_example.txt')
    if '/ext-zlib/gzopen_basic2.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzopen_basic2.gz')
    if '/ext-zlib/gzwrite_error.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzwrite_error.gz')
    if '/ext-zlib/gzwrite_error2.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzwrite_error2.gz')
    if '/ext-zlib/zlib_wrapper_fflush_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'zlib_wrapper_fflush_basic.gz')
    if '/ext-zlib/zlib_wrapper_ftruncate_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'zlib_wrapper_ftruncate_basic.gz')
    if '/ext-zlib/gzeof_variation1.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzeof_variation1.gz')
    if '/ext-zlib/gzputs_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzputs_basic.gz')
    if '/ext-zlib/gzread_variation1.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzread_variation1.gz')
    if '/ext-zlib/gzwrite_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzwrite_basic.gz')
    if '/ext-spl/bug42364.php' in full_dest_filename:
        test = test.replace('dirname(__FILE__)', '__DIR__."/../../../sample_dir/"')
    if '/ext-standard-file/lchgrp_basic.php' in full_dest_filename:
        test = test.replace('symlink.txt', 'lchgrp_basic_symlink.txt')
    if '/ext-standard-file/tempnam_variation5.php' in full_dest_filename:
        test = test.replace('tempnam_variation6', 'tempnam_variation5')
    if '/ext-standard-general_functions/bug41445_1.php' in full_dest_filename:
        test = test.replace('bug41445.ini', 'bug41445_1.ini')
    if '/ext-standard-file/bug24482.php' in full_dest_filename:
        test = test.replace('"*"', '__DIR__."/../../../sample_dir/("')
        test = test.replace('opendir(".")', 'opendir(__DIR__."/../../../sample_dir/")')

    file(full_dest_filename, 'w').write(test)

if args.zend_path:
    test_dirs = (('Zend/tests'), ('tests'), ('sapi'), ('ext'))
    def should_import(filename):
        for bad in no_import:
            if bad in filename:
                return False
        return True

    for source in test_dirs:
        for root, dirs, files in os.walk(os.path.join(args.zend_path, source)):
            for filename in files:
                full_file = os.path.join(root, filename)

                def matches(regexes):
                    if not regexes:
                        return True
                    for regex in regexes:
                        if re.search(regex, full_file):
                            return True
                    return False

                if matches(args.only) and should_import(full_file):
                    walk(full_file, root.replace(args.zend_path, ''))

if not os.path.isdir('test/zend/all'):
    if args.zend_path:
        print "No test/zend/all. Maybe no tests were imported?"
        sys.exit(0)
    else:
        print "Running all tests from test/zend/bad"
        shutil.copytree('test/zend/bad', 'test/zend/all')
else:
    print "Running all tests from zend/all"

file('test/zend/hphp_config.hdf', 'w').write('')
file('test/zend/config.hdf', 'w').write(
    'Eval {\n EnableObjDestructCall = true\n }'
)

stdout = subprocess.Popen(
    [
        'tools/verify_to_json.php',
        'test/run',
        'test/zend/all',
        'interp',
        '',
        '../_bin',
    ],
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT
).communicate()[0]

# segfaults also print on stderr
stdout = re.sub('\nsh: line 1:.*', '', stdout)
# fbmake, you are crazy
results = json.loads('['+stdout.strip().replace("\n", ",\n")+']')[-1]['results']

if args.verbose:
    print results

for test in results:
    filename = test['name']
    good_file = filename.replace('all', 'good', 1)
    bad_file = filename.replace('all', 'bad', 1)
    mkdir_p(os.path.dirname(good_file))
    mkdir_p(os.path.dirname(bad_file))

    good = (test['status'] == 'passed')
    for test in bad_tests:
        if test in filename:
            good = False

    if good:
        dest_file = good_file
        delete_file = bad_file
        subpath = 'good'
    else:
        dest_file = bad_file
        delete_file = good_file
        subpath = 'bad'

    exps = glob.glob(filename+'.expect*')
    if not exps:
        # this file is probably generated while running tests :(
        continue

    source_file_exp = exps[0]
    _, dest_ext = os.path.splitext(source_file_exp)
    os.rename(filename, dest_file)
    file(dest_file+dest_ext, 'w').write(
        file(source_file_exp).read().replace('/all', '/' + subpath)
    )
    os.unlink(source_file_exp)
    for f in glob.glob(delete_file+"*"):
        os.unlink(f)

# extra random files needed for tests...
for root, dirs, files in os.walk('test/zend/all'):
    for filename in files:
        filename = os.path.join(root, filename)

        for name in other_files:
            if name in filename:
                dest = filename.replace('all', 'good', 1)
                dir = os.path.dirname(dest)
                mkdir_p(dir)
                shutil.copyfile(filename, dest)

if not args.dirty:
    shutil.rmtree('test/zend/all')
