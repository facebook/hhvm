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
    '/Zend/tests/bug64660.phpt',

    # intermittent segfaults
    '/Zend/tests/001.phpt',
    '/Zend/tests/002.phpt',
    '/Zend/tests/003.phpt',

    # too large input
    '/tests/lang/024.phpt',

    # spews files until they work
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
    '/ext/spl/examples/',
    '/ext/xmlwriter/examples/',

    # not implemented extensions
    '/ext/com_dotnet',
    '/ext/dba',
    '/ext/dom',
    '/ext/enchant',
    '/ext/ereg',
    '/ext/fileinfo',
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
    '/ext/pspell',
    '/ext/readline',
    '/ext/recode',
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

    # conscious decision not to match these
    '/ext/spl/tests/arrayObject_getIteratorClass_basic1.phpt',

    # Zend's "run-tests" uses the cgi-fcgi sapi to run tests
    # our implementation is simply different by design
    '/ext/standard/versioning/php_sapi_name_variation001.phpt',
)

# For marking tests as always failing. Used to keep flaky tests in bad/.
bad_tests = (
    # SESSION is bused on husdon
    '/Zend/tests/unset_cv05.php',
    '/Zend/tests/unset_cv06.php',

    # works in interp but not others
    '/tests/lang/bug25922.php',
    '/Zend/tests/bug34064.php',
    '/Zend/tests/objects_029.php',
    '/Zend/tests/objects_030.php',
    '/ext/standard/tests/strings/bug44242.php',
    '/ext/standard/tests/streams/bug64770.php',
    '/ext/sqlite3/tests/sqlite3_29_createfunction.php',

    # line number is inconsistent on stack overflow
    '/Zend/tests/bug41633_3.php',

    # doubles aren't serialized properly in the repo
    '/ext/filter/tests/049.php',

    # broken in Jenkins
    '/ext/standard/array/tests/array_diff_assoc_variation6.php',
    '/Zend/tests/bug35239.php',
    '/Zend/tests/bug54265.php',

    # broken in contbuild
    '/ext/standard/tests/strings/bug51059.php',
    '/ext/standard/tests/strings/setlocale_variation1.php',
    '/ext/standard/tests/strings/setlocale_basic1.php',
    '/ext/standard/tests/strings/setlocale_basic2.php',
    '/ext/standard/tests/strings/setlocale_basic3.php',
    '/ext/standard/tests/file/filetype_variation2.php',
    '/ext/standard/tests/file/filetype_variation3.php',
    '/ext/sockets/tests/ipv4loop.php',
    '/ext/posix/tests/posix_kill_basic.php',
    '/ext/standard/tests/file/005_variation-win32.php',
    '/ext/json/tests/fail001.php',
    '/ext/standard/network/tests/getmxrr.php',
    '/ext/standard/network/tests/gethostbyname_error004.php',
    '/ext/standard/tests/file/fgets_socket_variation1.php',

    # our build machines have no members in group 0...
    '/ext/posix/tests/posix_getgrgid.php',
    '/ext/posix/tests/posix_getgroups_basic.php',

    # concurrency issues
    '/ext/ftp/tests/ftp_chmod_basic.php',
    '/ext/mysql/tests/001.php',
    '/ext/mysql/tests/bug47438.php',
    '/ext/mysql/tests/mysql_client_encoding.php',
    '/ext/mysql/tests/mysql_select_db.php',
    '/ext/sqlite3/tests/sqlite3_08_udf.php',
    '/ext/sqlite3/tests/sqlite3_25_create_aggregate.php',
    '/ext/sockets/tests/socket_connect_params.php',
    '/ext/sockets/tests/socket_getpeername_ipv4loop.php',
    '/ext/standard/tests/file/fread_socket_variation1.php',
    '/ext/standard/tests/network/fsockopen_variation1.php',
    '/ext/standard/tests/network/shutdown.php',

    # time tests are hard to write, these are poorly written
    '/ext/date/tests/bug48187.php',

    # broken on ubuntu 12.04
    '/ext/date/tests/DateTimeZone_listAbbreviations_basic1.php',
    '/ext/date/tests/bug52290.php',
    '/ext/date/tests/timezone_abbreviations_list_basic1.php',
    '/ext/gd/tests/bug24155.php',
    '/ext/gd/tests/bug39366.php',
    '/ext/gd/tests/bug48732.php',
    '/ext/mbstring/tests/bug54494.php',
    '/ext/openssl/tests/openssl_x509_parse_basic.php',
    '/ext/standard/tests/streams/bug61115-2.php',
    '/ext/standard/tests/strings/moneyformat.php',

    # works in interp but fails in JIT
    '/ext/standard/tests/array/array_next_error2.php',
    '/ext/standard/tests/array/prev_error3.php',
    '/ext/standard/tests/class_object/get_object_vars_variation_003.php',
    '/ext/standard/tests/file/pathinfo_variation1.php',
    '/ext/standard/tests/file/pathinfo_variaton.php',
    '/tests/lang/038.php',
    '/tests/lang/045.php',
    '/Zend/tests/lsb_021.php',
    '/Zend/tests/lsb_022.php',


    # flaky for various reasons
    '/ext/standard/tests/network/gethostbyname_error004.php',
    '/ext/standard/tests/network/getmxrr.php',
    '/ext/sockets/tests/socket_getpeername_ipv6loop.php',

    # broken: t2991109
    '/ext/zlib/tests/gzfile_variation5.php',
    '/ext/zlib/tests/readgzfile_variation5.php',
)

# Tests that work but not in repo mode
norepo_tests = (
    # TODO: See if any of these should work in repo mode
    '/Zend/tests/014.php',
    '/Zend/tests/035.php',
    '/Zend/tests/bug26697.php',
    '/Zend/tests/bug28444.php',
    '/Zend/tests/bug30519.php',
    '/Zend/tests/bug30922.php',
    '/Zend/tests/bug36006.php',
    '/Zend/tests/bug36759.php',
    '/Zend/tests/bug39542.php',
    '/Zend/tests/bug43651.php',
    '/Zend/tests/bug44141.php',
    '/Zend/tests/bug55578.php',
    '/Zend/tests/bug63741.php',
    '/Zend/tests/class_alias_013.php',
    '/Zend/tests/class_constants_003.php',
    '/Zend/tests/class_exists_001.php',
    '/Zend/tests/constants_005.php',
    '/Zend/tests/errmsg_007.php',
    '/Zend/tests/errmsg_026.php',
    '/Zend/tests/errmsg_035.php',
    '/Zend/tests/errmsg_036.php',
    '/Zend/tests/error_reporting04.php',
    '/Zend/tests/error_reporting09.php',
    '/Zend/tests/jump14.php',
    '/Zend/tests/lsb_013.php',
    '/Zend/tests/ns_041.php',
    '/Zend/tests/traits/bug60369.php',
    '/Zend/tests/traits/bug60809.php',
    '/Zend/tests/traits/bugs/overridding-conflicting-property-initializer.php',
    '/Zend/tests/traits/error_003.php',
    '/Zend/tests/traits/property003.php',
    '/Zend/tests/traits/property004.php',
    '/Zend/tests/unset_cv01.php',
    '/ext/bz2/tests/with_strings.php',
    '/ext/pdo_sqlite/tests/bug33841.php',
    '/ext/pdo_sqlite/tests/bug46139.php',
    '/ext/pdo_sqlite/tests/bug52487.php',
    '/ext/sqlite3/tests/bug47159.php',
    '/ext/sqlite3/tests/sqlite3_01_open.php',
    '/ext/sqlite3/tests/sqlite3_02_create.php',
    '/ext/sqlite3/tests/sqlite3_03_insert.php',
    '/ext/sqlite3/tests/sqlite3_04_update.php',
    '/ext/sqlite3/tests/sqlite3_05_delete.php',
    '/ext/sqlite3/tests/sqlite3_09_blob_bound_param.php',
    '/ext/sqlite3/tests/sqlite3_13_skip_all_cleanup.php',
    '/ext/sqlite3/tests/sqlite3_14_querysingle.php',
    '/ext/sqlite3/tests/sqlite3_16_select_no_results.php',
    '/ext/sqlite3/tests/sqlite3_18_changes.php',
    '/ext/sqlite3/tests/sqlite3_19_columninfo.php',
    '/ext/sqlite3/tests/sqlite3_20_error.php',
    '/ext/sqlite3/tests/sqlite3_24_last_insert_rowid.php',
    '/ext/sqlite3/tests/sqlite3stmt_paramCount_basic.php',
    '/ext/sqlite3/tests/sqlite3stmt_paramCount_error.php',
    '/ext/standard/tests/class_object/class_exists_basic_001.php',
    '/ext/standard/tests/class_object/get_declared_classes_variation1.php',
    '/ext/standard/tests/class_object/get_declared_interfaces_variation1.php',
    '/ext/standard/tests/class_object/get_declared_traits_variation1.php',
    '/ext/standard/tests/class_object/interface_exists_variation3.php',
    '/ext/standard/tests/class_object/interface_exists_variation4.php',
    '/ext/standard/tests/file/file_get_contents_basic.php',
    '/ext/standard/tests/file/file_get_contents_file_put_contents_basic.php',
    '/ext/standard/tests/file/file_get_contents_file_put_contents_variation1.php',
    '/ext/standard/tests/file/file_get_contents_file_put_contents_variation2.php',
    '/ext/standard/tests/file/file_get_contents_variation1.php',
    '/ext/standard/tests/general_functions/is_numeric.php',
    '/ext/standard/tests/math/abs.php',
    '/ext/standard/tests/math/pow.php',
    '/ext/standard/tests/serialize/bug30234.php',
    '/ext/standard/tests/strings/trim.php',
    '/ext/standard/tests/url/parse_url_basic_002.php',
    '/ext/standard/tests/url/parse_url_basic_003.php',
    '/ext/standard/tests/url/parse_url_basic_004.php',
    '/ext/standard/tests/url/parse_url_basic_005.php',
    '/ext/standard/tests/url/parse_url_basic_006.php',
    '/ext/standard/tests/url/parse_url_basic_007.php',
    '/ext/standard/tests/url/parse_url_basic_008.php',
    '/ext/standard/tests/url/parse_url_basic_009.php',
    '/tests/classes/autoload_001.php',
    '/tests/classes/autoload_002.php',
    '/tests/classes/autoload_003.php',
    '/tests/classes/autoload_004.php',
    '/tests/classes/autoload_005.php',
    '/tests/classes/autoload_006.php',
    '/tests/classes/autoload_010.php',
    '/tests/classes/autoload_018.php',
    '/tests/classes/constants_scope_001.php',
    '/tests/lang/static_variation_001.php',
    '/tests/lang/static_variation_002.php',
)

# Random other files that zend wants
other_files = (
    '/Zend/tests/014.inc',
    '/Zend/tests/bug39542/bug39542.php',
    '/Zend/tests/bug46665_autoload.inc',
    '/Zend/tests/bug54804.inc',
    '/Zend/tests/constants/fixtures/folder1/fixture.php',
    '/Zend/tests/constants/fixtures/folder1/subfolder1/fixture.php',
    '/Zend/tests/constants/fixtures/folder1/subfolder2/fixture.php',
    '/Zend/tests/constants/fixtures/folder1/subfolder3/fixture.php',
    '/Zend/tests/constants/fixtures/folder1/subfolder4/fixture.php',
    '/Zend/tests/constants/fixtures/folder2/fixture.php',
    '/Zend/tests/constants/fixtures/folder2/subfolder1/fixture.php',
    '/Zend/tests/constants/fixtures/folder2/subfolder2/fixture.php',
    '/Zend/tests/constants/fixtures/folder2/subfolder3/fixture.php',
    '/Zend/tests/constants/fixtures/folder2/subfolder4/fixture.php',
    '/Zend/tests/constants/fixtures/folder3/fixture.php',
    '/Zend/tests/constants/fixtures/folder3/subfolder1/fixture.php',
    '/Zend/tests/constants/fixtures/folder3/subfolder2/fixture.php',
    '/Zend/tests/constants/fixtures/folder3/subfolder3/fixture.php',
    '/Zend/tests/constants/fixtures/folder3/subfolder4/fixture.php',
    '/Zend/tests/constants/fixtures/folder4/fixture.php',
    '/Zend/tests/constants/fixtures/folder4/subfolder1/fixture.php',
    '/Zend/tests/constants/fixtures/folder4/subfolder2/fixture.php',
    '/Zend/tests/constants/fixtures/folder4/subfolder3/fixture.php',
    '/Zend/tests/constants/fixtures/folder4/subfolder4/fixture.php',
    '/Zend/tests/nowdoc.inc',
    '/Zend/tests/ns_022.inc',
    '/Zend/tests/ns_027.inc',
    '/Zend/tests/ns_028.inc',
    '/Zend/tests/ns_065.inc',
    '/Zend/tests/ns_067.inc',
    '/Zend/tests/ns_069.inc',
    '/Zend/tests/unset.inc',
    '/ext/calendar/tests/skipif.inc',
    '/ext/curl/tests/curl_testdata1.txt',
    '/ext/curl/tests/curl_testdata2.txt',
    '/ext/date/tests/DateTime_data-absolute.inc',
    '/ext/date/tests/DateTime_data-dates.inc',
    '/ext/date/tests/DateTime_data-fall-type2-type2.inc',
    '/ext/date/tests/DateTime_data-fall-type2-type3.inc',
    '/ext/date/tests/DateTime_data-fall-type3-type2.inc',
    '/ext/date/tests/DateTime_data-fall-type3-type3.inc',
    '/ext/date/tests/DateTime_data-february.inc',
    '/ext/date/tests/DateTime_data-massive.inc',
    '/ext/date/tests/DateTime_data-spring-type2-type2.inc',
    '/ext/date/tests/DateTime_data-spring-type2-type3.inc',
    '/ext/date/tests/DateTime_data-spring-type3-type2.inc',
    '/ext/date/tests/DateTime_data-spring-type3-type3.inc',
    '/ext/date/tests/examine_diff.inc',
    '/ext/exif/tests/bug48378.jpeg',
    '/ext/ftp/tests/cert.pem',
    '/ext/ftp/tests/server.inc',
    '/ext/gd/tests/Tuffy.ttf',
    '/ext/gd/tests/bug37346.gif',
    '/ext/gd/tests/bug38112.gif',
    '/ext/gd/tests/bug43121.gif',
    '/ext/gd/tests/conv_test.gif',
    '/ext/gd/tests/conv_test.jpeg',
    '/ext/gd/tests/conv_test.png',
    '/ext/gd/tests/conv_test.xbm',
    '/ext/gd/tests/php.gif',
    '/ext/gd/tests/src.gd2',
    '/ext/gd/tests/src.wbmp',
    '/ext/gd/tests/test8859.ttf',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest.mo',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest.po',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest_switch.mo',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest_switch.po',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest_switched.po',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dngettextTest.mo',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dngettextTest.po',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/dgettextTest.mo',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/dgettextTest.po',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/dgettextTest_switch.mo',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/dgettextTest_switch.po',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/dngettextTest.mo',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/dngettextTest.po',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/messages.mo',
    '/ext/gettext/tests/locale/en/LC_MESSAGES/messages.po',
    '/ext/gettext/tests/locale/fi/LC_MESSAGES/messages.mo',
    '/ext/gettext/tests/locale/fi/LC_MESSAGES/messages.po',
    '/ext/intl/tests/ut_common.inc',
    '/ext/ldap/tests/connect.inc',
    '/ext/mbstring/tests/common.inc',
    '/ext/mcrypt/tests/vectors.txt',
    '/ext/mysql/tests/connect.inc',
    '/ext/openssl/tests/005_crt.txt',
    '/ext/openssl/tests/bug28382cert.txt',
    '/ext/openssl/tests/bug37820cert.pem',
    '/ext/openssl/tests/bug37820key.pem',
    '/ext/openssl/tests/bug39217cert1.txt',
    '/ext/openssl/tests/bug39217cert2.txt',
    '/ext/openssl/tests/bug41033.pem',
    '/ext/openssl/tests/bug41033pub.pem',
    '/ext/openssl/tests/cert.crt',
    '/ext/openssl/tests/openssl.cnf',
    '/ext/openssl/tests/private.key',
    '/ext/openssl/tests/public.key',
    '/ext/pdo/tests/pdo_test.inc',
    '/ext/pdo_mysql/tests/common.phpt',
    '/ext/pdo_mysql/tests/config.inc',
    '/ext/pdo_sqlite/tests/common.phpt',
    '/ext/session/tests/save_handler.inc',
    '/ext/simplexml/tests/book.xml',
    '/ext/simplexml/tests/bug24392.xml',
    '/ext/soap/tests/bugs/bug30928.wsdl',
    '/ext/soap/tests/bugs/bug41337.wsdl',
    '/ext/soap/tests/bugs/bug42692.wsdl',
    '/ext/soap/tests/schema/test_schema.inc',
    '/ext/soap/tests/soap12/soap12-test.inc',
    '/ext/soap/tests/soap12/soap12-test.wsdl',
    '/ext/spl/tests/SplFileObject_testinput.csv',
    '/ext/spl/tests/fileobject_001a.txt',
    '/ext/spl/tests/fileobject_001b.txt',
    '/ext/spl/tests/testclass.class.inc',
    '/ext/sqlite3/tests/new_db.inc',
    '/ext/sqlite3/tests/stream_test.inc',
    '/ext/standard/tests/array/compare_function.inc',
    '/ext/standard/tests/class_object/AutoInterface.inc',
    '/ext/standard/tests/class_object/AutoLoaded.inc',
    '/ext/standard/tests/class_object/AutoTrait.inc',
    '/ext/standard/tests/file/bug40501.csv',
    '/ext/standard/tests/file/file.inc',
    '/ext/standard/tests/file/fopen_include_path.inc',
    '/ext/standard/tests/file/test.csv',
    '/ext/standard/tests/file/test2.csv',
    '/ext/standard/tests/file/test3.csv',
    '/ext/standard/tests/general_functions/004.data',
    '/ext/standard/tests/image/246x247.png',
    '/ext/standard/tests/image/384x385.png',
    '/ext/standard/tests/image/bug13213.jpg',
    '/ext/standard/tests/image/test13pix.swf',
    '/ext/standard/tests/image/test1pix.jpg',
    '/ext/standard/tests/image/testAPP.jpg',
    '/ext/standard/tests/serialize/autoload_implements.p5c',
    '/ext/standard/tests/serialize/autoload_interface.p5c',
    '/ext/standard/tests/url/urls.inc',
    '/ext/xml/tests/xmltest.xml',
    '/ext/xmlreader/tests/012.dtd',
    '/ext/xmlreader/tests/012.xml',
    '/ext/xmlreader/tests/dtdexample.dtd',
    '/ext/xmlreader/tests/relaxNG.rng',
    '/ext/xmlreader/tests/relaxNG2.rng',
    '/ext/xmlreader/tests/relaxNG3.rng',
    '/ext/zlib/tests/004.txt.gz',
    '/tests/classes/autoload_derived.p5c',
    '/tests/classes/autoload_implements.p5c',
    '/tests/classes/autoload_interface.p5c',
    '/tests/classes/autoload_root.p5c',
    '/tests/classes/constants_basic_003.inc',
    '/tests/classes/interface_optional_arg_003.inc',
    '/tests/lang/015.inc',
    '/tests/lang/016.inc',
    '/tests/lang/023-2.inc',
    '/tests/lang/inc.inc',
    '/tests/lang/inc_throw.inc',
    '/tests/lang/include_files/echo.inc',
    '/tests/lang/include_files/function.inc',
    '/tests/quicktester.inc',
)

parser = argparse.ArgumentParser()
parser.add_argument(
    "-z",
    "--zend_path",
    type=str,
    required=True,
    help="zend path to import tests from. git clone https://github.com/php/php-src"
)
parser.add_argument(
    "-o",
    "--only",
    type=str,
    action='append',
    help="only import tests whose path contains this substring."
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

def walk(filename, source_dir):
    dest_filename = os.path.basename(filename)

    cur_dir = os.path.dirname(__file__)
    dest_subdir = os.path.join(cur_dir, '../test/zend/all', source_dir)
    mkdir_p(dest_subdir)
    full_dest_filename = os.path.join(dest_subdir, dest_filename)

    # Exactly mirror zend's directories incase some tests depend on random crap.
    # We'll only move things we want into 'good'
    shutil.copyfile(filename, full_dest_filename)

    full_dest_filename = full_dest_filename.replace('.phpt', '.php')

    if not '.phpt' in filename:
        def replace(find, replace):
            data = file(full_dest_filename).read().replace(find, replace)
            file(full_dest_filename, 'w').write(data)

        if '/ext/ftp/tests/server.inc' in full_dest_filename:
            replace('stream_socket_server', '@stream_socket_server')

        if full_dest_filename.endswith('.php'):
            f = file(full_dest_filename.replace('.php', '.php.skipif'), 'w')
            f.write('always skip - not a test')

        return

    print "Importing %s" % full_dest_filename

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

            # PHP puts a newline in that we don't
            exp = exp.replace('\n\nFatal error:', '\nFatal error:')
            exp = exp.replace('\n\nCatchable fatal error:', '\nCatchable fatal error:')
            exp = exp.replace('\n\nWarning:', '\nWarning:')
            exp = exp.replace('\n\nNotice:', '\nNotice:')

            match_rest_of_line = '%s'
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

        if '/ext/standard/tests/file/tempnam_variation5.php' in full_dest_filename:
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
            '$_REQUEST = array_merge($_REQUEST, $_POST);\n'
        )
    if sections.has_key('GET'):
        test = test.replace(
            '<?php',
            '<?php\nparse_str("' + sections['GET'] + '", $_GET);\n'
            '$_REQUEST = array_merge($_REQUEST, $_GET);\n'
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
        if not test.strip().endswith('?>'):
            test += '?>\n'
        test += sections['CLEAN']

    # If you put an exception in here, please send a pull request upstream to
    # php-src. Then when it gets merged kill your hack.
    if '/Zend/tests/bug60771.php' in full_dest_filename:
        test = test.replace("?>", "unlink('test.php');\n?>")
    if '/ext/standard/tests/file/bug44805.php' in full_dest_filename:
        test = test.replace("1)) {\n\tunlink($file2", "2)) {\n\tunlink($file2")
    if '/ext/bz2/tests/bug51997.php' in full_dest_filename:
        test = test.replace("testfile.bz2", "bug51997.bz2")
    if '/ext/bz2/tests/with_files.php' in full_dest_filename:
        test = test.replace("testfile.bz2", "with_files.bz2")
    if ('/ext/standard/tests/math/pow.php' in full_dest_filename) or \
       ('/ext/standard/tests/math/abs.php' in full_dest_filename):
        # HHVM handles int->double promotion differently than Zend
        # But the pow() function behaves the same, so we have to fudge it a bit
        test = test.replace("9223372036854775807",
                            "(double)9223372036854775807")
        test = test.replace("0x7FFFFFFF", "(double)0x7FFFFFFF")
        test = test.replace("is_int(LONG_MIN  )", "is_float(LONG_MIN  )")
        test = test.replace("is_int(LONG_MAX  )", "is_float(LONG_MAX  )")
    if '/ext/xmlreader/tests/003.php' in full_dest_filename:
        test = test.replace("_002.xml", "_003.xml")
    if '/ext/xmlreader/tests/004.php' in full_dest_filename:
        test = test.replace("_002.xml", "_004.xml")
    if '/ext/standard/tests/file/bug45181.php' in full_dest_filename:
        test = test.replace('rmdir("bug45181_x");',
                'chdir("..");\nrmdir("bug45181_x");')
    if '/ext/standard/tests/file/bug53848.php' in full_dest_filename:
        test = test.replace('bug39538.csv', 'bug53848.csv')
    if '/ext/zlib/tests/bug61139.php' in full_dest_filename:
        test += "\nunlink('someFile');\n?>"
    if '/ext/zlib/tests/gzseek_variation7.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation7.gz')
    if '/ext/zlib/tests/gzseek_basic2.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_basic2.gz')
    if '/ext/zlib/tests/gzseek_variation1.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation1.gz')
    if '/ext/zlib/tests/gzseek_variation4.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation4.gz')
    if '/ext/zlib/tests/gzseek_variation5.php' in full_dest_filename:
        test = test.replace('temp3.txt.gz', 'gzseek_variation5.gz')
    if '/ext/standard/tests/strings/vfprintf_' in full_dest_filename:
        test = test.replace('dump.txt', dest_filename + '.txt')
    if '/ext/standard/tests/file/touch_basic.php' in full_dest_filename:
        test = test.replace('touch.dat', 'touch_basic.dat')
    if '/ext/standard/tests/file/touch_variation2.php' in full_dest_filename:
        test = test.replace('touch.dat', 'touch_variation2.dat')
    if '/ext/standard/tests/file/file_put_contents_variation9.php' in full_dest_filename:
        test = test.replace('fileGetContentsVar9', 'filePutContentsVar9')
    if '/ext/standard/tests/network/tcp4loop.php' in full_dest_filename:
        test = test.replace('31337', '31338')
    if '/ext/standard/tests/file/fpassthru_variation.php' in full_dest_filename:
        test = test.replace('passthru.tmp', 'fpassthru_variation.tmp')
    if '/ext/spl/tests/SplFileInfo_getPerms_basic.php' in full_dest_filename:
        test = test.replace('test_file_ptfi', 'SplFileInfo_getPerms_basic.txt')
    if '/ext/spl/tests/SplFileInfo_getInode_basic.php' in full_dest_filename:
        test = test.replace('test_file_ptfi', 'SplFileInfo_getInode_basic.txt')
    if '/ext/standard/tests/file/mkdir-001.php' in full_dest_filename:
        test = test.replace('testdir', 'mkdir-001')
    if '/ext/standard/tests/file/mkdir-002.php' in full_dest_filename:
        test = test.replace('testdir', 'mkdir-002')
    if '/ext/standard/tests/file/mkdir-003.php' in full_dest_filename:
        test = test.replace('testdir', 'mkdir-003')
    if '/ext/standard/tests/file/filesize_variation3-win32.php' in full_dest_filename:
        test = test.replace('filesize_variation3', 'filesize_variation3-win32')
    if '/ext/hash/tests/hash_file_basic.php' in full_dest_filename:
        test = test.replace('hash_file_example.txt', 'hash_file_basic_example.txt')
    if '/ext/hash/tests/hash_file_error.php' in full_dest_filename:
        test = test.replace('hash_file_example.txt', 'hash_file_error_example.txt')
    if '/ext/zlib/tests/gzopen_basic2.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzopen_basic2.gz')
    if '/ext/zlib/tests/gzwrite_error.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzwrite_error.gz')
    if '/ext/zlib/tests/gzwrite_error2.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzwrite_error2.gz')
    if '/ext/zlib/tests/zlib_wrapper_fflush_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'zlib_wrapper_fflush_basic.gz')
    if '/ext/zlib/tests/zlib_wrapper_ftruncate_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'zlib_wrapper_ftruncate_basic.gz')
    if '/ext/zlib/tests/gzeof_variation1.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzeof_variation1.gz')
    if '/ext/zlib/tests/gzputs_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzputs_basic.gz')
    if '/ext/zlib/tests/gzread_variation1.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzread_variation1.gz')
    if '/ext/zlib/tests/gzwrite_basic.php' in full_dest_filename:
        test = test.replace('temp.txt.gz', 'gzwrite_basic.gz')
    if '/ext/spl/tests/bug42364.php' in full_dest_filename:
        test = test.replace('dirname(__FILE__)', '__DIR__."/../../../../../sample_dir/"')
    if '/ext/standard/tests/file/lchgrp_basic.php' in full_dest_filename:
        test = test.replace('symlink.txt', 'lchgrp_basic_symlink.txt')
    if '/ext/standard/tests/file/tempnam_variation5.php' in full_dest_filename:
        test = test.replace('tempnam_variation6', 'tempnam_variation5')
    if '/ext/standard/tests/general_functions/bug41445_1.php' in full_dest_filename:
        test = test.replace('bug41445.ini', 'bug41445_1.ini')
    if '/ext/standard/tests/file/bug24482.php' in full_dest_filename:
        test = test.replace('"*"', '__DIR__."/../../../../../../sample_dir/*"')
        test = test.replace('opendir(".")', 'opendir(__DIR__."/../../../../../../sample_dir/")')
        test = test.replace('is_dir($file)', 'is_dir(__DIR__."/../../../../../../sample_dir/".$file)')
    if '/ext/spl/tests/SplFileObject_fgetcsv_' in full_dest_filename:
        test = test.replace('SplFileObject__fgetcsv.csv',
            os.path.basename(full_dest_filename).replace('.php', '.csv'))
    if '/ext/spl/tests/SplFileObject_' in full_dest_filename:
        test = test.replace('testdata.csv',
            os.path.basename(full_dest_filename).replace('.php', '.csv'))
    if '/ext/spl/tests/SplFileObject_rewind_error001.php' in full_dest_filename:
        test = test.replace("?>", "unlink('SplFileObject_rewind_error001.csv');\n?>")
    if '/ext/zlib/tests/gzfile_basic.php' in full_dest_filename:
        test = test.replace("plainfile.txt.gz", "gzfile_basic.txt.gz")
    if '/ext/zlib/tests/gzfile_basic2.php' in full_dest_filename:
        test = test.replace("plainfile.txt", "gzfile_basic2.txt")
    if '/ext/standard/tests/network/fsockopen_variation1.php' in full_dest_filename:
        test = test.replace("<?php", "<?php\n$port = rand(50000, 65535);")
        test = test.replace("31337'", "'.$port")
    if '/ext/standard/tests/network/shutdown.php' in full_dest_filename:
        test = test.replace("<?php", "<?php\n$port = rand(50000, 65535);")
        test = test.replace("31337'", "'.$port")
    if '/ext/standard/tests/file/fgets_socket_variation1.php' in full_dest_filename:
        test = test.replace("<?php", "<?php\n$port = rand(50000, 65535);")
        test = test.replace("31337'", "'.$port")
    if '/ext/standard/tests/file/fread_socket_variation1.php' in full_dest_filename:
        test = test.replace("<?php", "<?php\n$port = rand(50000, 65535);")
        test = test.replace("31337'", "'.$port")
    if '/ext/standard/tests/file/fputcsv.php' in full_dest_filename:
        test = test.replace("fgetcsv.csv", "fputcsv.csv")
    if '/ext/spl/tests/SplFileObject_fputcsv_' in full_dest_filename:
        test = test.replace('SplFileObject_fputcsv.csv',
            os.path.basename(full_dest_filename).replace('.php', '.csv'))
    if '/ext/ftp/tests/ftp_' in full_dest_filename:
        test = test.replace('localfile.txt',
            os.path.basename(full_dest_filename).replace('.php', '.txt'))
    if '/Zend/tests/bug36759.php' in full_dest_filename:
        pseudomain = '$y = new Bar();\n$x = new Foo($y);\n'
        test = test.replace(pseudomain,
                            'function main() {\n' + pseudomain + '}\nmain();\n')


    file(full_dest_filename, 'w').write(test)

def should_import(filename):
    for bad in no_import:
        if bad in filename:
            return False
    return True

for root, dirs, files in os.walk(args.zend_path):
    for filename in files:
        full_file = os.path.join(root, filename)

        def matches(patterns):
            if not patterns:
                return True
            for pattern in patterns:
                if pattern in full_file:
                    return True
            return False

        if matches(args.only) and should_import(full_file):
            walk(full_file, root.replace(args.zend_path, ''))

if not os.path.isdir('test/zend/all'):
    print "No test/zend/all. Maybe no tests were imported?"
    sys.exit(0)
else:
    print "Running all tests from zend/all"

stdout = subprocess.Popen(
    [
        'test/run',
        '--fbmake',
        '-m',
        'interp',
        'test/zend/all',
    ],
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT
).communicate()[0]

# segfaults also print on stderr
stdout = re.sub('\nsh: line 1:.*', '', stdout)
# just the last line
last_line = stdout.strip().split("\n")[-1]
results = json.loads(last_line)['results']

if args.verbose:
    print results
else:
    print "Test run done, moving files"

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

    needs_norepo = False
    if good:
        dest_file = good_file
        delete_file = bad_file
        subpath = 'good'
        for test in norepo_tests:
            if test in filename:
                needs_norepo = True
    else:
        dest_file = bad_file
        delete_file = good_file
        subpath = 'bad'

    exps = glob.glob(filename + '.expect*')
    if not exps:
        # this file is probably generated while running tests :(
        continue

    source_file_exp = exps[0]
    _, dest_ext = os.path.splitext(source_file_exp)
    shutil.copyfile(filename, dest_file)
    file(dest_file + dest_ext, 'w').write(
        file(source_file_exp).read().replace('/all', '/' + subpath)
    )
    if needs_norepo:
        file(dest_file + '.norepo', 'w')
    if os.path.exists(filename + '.skipif'):
        shutil.copyfile(filename + '.skipif', dest_file + '.skipif')
    for f in glob.glob(delete_file + "*"):
        if os.path.isfile(f):
            os.unlink(f)
        else:
            shutil.rmtree(f)

# extra random files needed for tests...
for root, dirs, files in os.walk('test/zend/all'):
    for filename in files:
        filename = os.path.join(root, filename)

        for name in other_files:
            if name in filename:
                for subdir in ('good', 'bad'):
                    dest = filename.replace('all', subdir, 1)
                    dir = os.path.dirname(dest)
                    mkdir_p(dir)
                    shutil.copyfile(filename, dest)

if not args.dirty:
    shutil.rmtree('test/zend/all')
