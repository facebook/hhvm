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
import tarfile
import urllib2

# The version that we will be importing the tests from.
# Must be a valid, released version from php download site
zend_version = "5.6.1"

# Don't even pull these into the repo.
# We want running the bad tests to still complete.
no_import = (
    # these hang forever
    '/ext/standard/tests/array/array_pad_variation2.phpt',
    '/ext/standard/tests/general_functions/sleep_error.phpt',
    '/ext/standard/tests/general_functions/usleep_error.phpt',
    '/ext/zlib/tests/gzgetc_basic.phpt',
    '/ext/zlib/tests/gzgets_basic.phpt',

    # too large input
    '/tests/lang/024.phpt',

    # cause of failure uninvestigated:
    'ext/zlib/tests/gzreadgzwriteplain.php',
    'ext/spl/tests/SplFileObject_setCsvControl_variation001.php',

    # spews files until they work
    '/ext/standard/tests/file/readfile_variation4.phpt',
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
    '/ext/enchant',
    '/ext/ereg',
    '/ext/interbase',
    '/ext/mssql',
    '/ext/mysqlnd',
    '/ext/oci8',
    '/ext/odbc',
    '/ext/pdo_dblib',
    '/ext/pdo_firebird',
    '/ext/pdo_oci',
    '/ext/pdo_odbc',
    '/ext/pdo_pgsql',
    '/ext/pspell',
    '/ext/recode',
    '/ext/shmop',
    '/ext/skeleton',
    '/ext/snmp',
    '/ext/sybase_ct',
    '/ext/sysvmsg',
    '/ext/sysvsem',
    '/ext/sysvshm',
    '/ext/tidy',
    '/ext/wddx',
    '/ext/xmlrpc',
    '/sapi',

    # conscious decision not to match these
    '/ext/spl/tests/arrayObject_getIteratorClass_basic1.phpt',

    # Zend's "run-tests" uses the cgi-fcgi sapi to run tests
    # our implementation is simply different by design
    '/ext/standard/versioning/php_sapi_name_variation001.phpt',
)

# For marking tests as always failing. Used to keep flaky tests in flaky/.
flaky_tests = (
    # line number is inconsistent on stack overflow
    '/Zend/tests/bug41633_3.php',

    # "ls" sometimes prints on stderr
    '/ext/standard/tests/streams/bug64770.php',

    # our build machines have no members in group 0...
    '/ext/posix/tests/posix_getgrgid.php',

    # Checking stat of ' ' races with mkdir_variation1
    '/ext/standard/tests/file/lstat_stat_variation22.php',
    '/ext/standard/tests/file/copy_variation3.php',
    '/ext/standard/tests/file/file_exists_variation1.php',

    # concurrency issues
    '/ext/mysql/tests/001.php',
    '/ext/mysql/tests/bug47438.php',
    '/ext/mysql/tests/mysql_client_encoding.php',
    '/ext/mysql/tests/mysql_select_db.php',
    '/ext/sqlite3/tests/sqlite3_08_udf.php',
    '/ext/sqlite3/tests/sqlite3_25_create_aggregate.php',
    '/ext/standard/tests/file/bug41655_2.php',
    '/ext/standard/tests/file/disk_free_space_basic.php',
    '/ext/standard/tests/file/disk_total_space_basic.php',
    '/ext/standard/tests/file/disk_total_space_variation.php',
    '/ext/standard/tests/file/fread_socket_variation1.php',
    '/ext/standard/tests/file/symlink_link_linkinfo_is_link_variation3.php',
    '/ext/standard/tests/network/fsockopen_variation1.php',
    '/ext/standard/tests/network/shutdown.php',
    '/ext/standard/tests/strings/fprintf_variation_001.php',
    '/ext/spl/tests/RecursiveDirectoryIterator_getSubPathname_basic.php',
    '/ext/spl/tests/SplFileObject_fgetcsv_basic.php',
    '/ext/spl/tests/SplFileObject_fgetcsv_delimiter_basic.php',
    '/ext/spl/tests/SplFileObject_fgetcsv_escape_basic.php',
    '/ext/spl/tests/SplFileObject_fgetcsv_escape_default.php',
    '/ext/spl/tests/SplFileObject_fgetcsv_enclosure_basic.php',
    '/ext/spl/tests/SplFileObject_fgetcsv_enclosure_error.php',
    '/ext/spl/tests/SplFileObject_setCsvControl_error001.php',
    '/ext/spl/tests/SplFileObject_setCsvControl_error002.php',
    '/ext/spl/tests/SplFileObject_setCsvControl_error003.php',
    '/ext/phar/tests/019.php',
    '/ext/spl/tests/dit_006.php',

    # these tests use each other's data
    '/ext/standard/tests/file/bug38086.php',
    '/ext/standard/tests/file/stream_copy_to_stream.php',

    # these all write to temp3.txt.gz
    '/ext/zlib/tests/gzseek_basic2.php',
    '/ext/zlib/tests/gzseek_variation1.php',
    '/ext/zlib/tests/gzseek_variation4.php',
    '/ext/zlib/tests/gzseek_variation5.php',

    # broken on ubuntu 12.04
    '/ext/date/tests/DateTimeZone_listAbbreviations_basic1.php',
    '/ext/date/tests/bug52290.php',
    '/ext/date/tests/timezone_abbreviations_list_basic1.php',
    '/ext/standard/tests/streams/bug61115-2.php',

    # timing dependent
    '/ext/date/tests/bug48187.php',

    # works in interp but fails in JIT
    '/ext/standard/tests/array/array_next_error2.php',
    '/ext/standard/tests/array/prev_error3.php',
    '/ext/standard/tests/class_object/get_object_vars_variation_003.php',
    '/tests/lang/038.php',

    # flaky for various reasons
    '/ext/sockets/tests/socket_getpeername_ipv6loop.php',

    # segfaults on contbuild in opt
    '/ext/standard/tests/strings/explode_bug.php',

    # XSL
    '/ext/xsl/tests/bug49634.php',
    '/ext/xsl/tests/bug54446_with_ini.php',
    '/ext/xsl/tests/xsl-phpinfo.php',
    '/ext/xsl/tests/xslt009.php',
    '/ext/xsl/tests/xsltprocessor_getParameter-wrongparam.php',
    '/ext/xsl/tests/xsltprocessor_removeParameter-wrongparams.php',

    # flaky: t3619770
    '/ext/zlib/tests/gzfile_basic.php',
    '/ext/zlib/tests/readgzfile_basic.php',

    # flaky: t3817758
    '/ext/ftp/tests/ftp_nb_fget_basic1.php',

    # flaky: t3851970
    '/ext/sockets/tests/socket_write_params.php',
    '/ext/sockets/tests/socket_bind_params.php',
    '/ext/sockets/tests/socket_getpeername.php',
    '/ext/session/tests/009.php',
    '/ext/standard/tests/file/bug39538.php',
    '/ext/standard/tests/file/bug53848.php',
    '/ext/standard/tests/general_functions/proc_open02.php',
    '/ext/ftp/tests/bug39458.php',
    '/ext/standard/tests/file/rename_variation3.php',
    '/ext/standard/tests/file/mkdir-003.php',
    '/ext/standard/tests/file/symlink_link_linkinfo_is_link_variation9.php',
    '/ext/zlib/tests/gzfile_basic.php',
    '/ext/standard/tests/file/fopen_variation12.php',
    '/ext/standard/tests/file/mkdir-002.php',
    '/ext/standard/tests/file/readlink_realpath_variation1.php',
    '/ext/session/tests/027.php',
    '/ext/standard/tests/file/lchgrp_basic.php',
    '/ext/standard/tests/file/mkdir-001.php',
    '/ext/sockets/tests/ipv4loop.php',
    '/ext/ftp/tests/ftp_alloc_basic1.php',
    '/ext/ftp/tests/ftp_alloc_basic2.php',

    # flaky on Travis: t4088096
    '/ext/curl/tests/curl_copy_handle_basic_006.php',
    '/ext/curl/tests/curl_copy_handle_basic_007.php',

    # a new process can crop up
    '/ext/posix/tests/posix_kill_basic.php',

    # Using PHP7 versions of these tests
    '/ext/session/tests/session_set_save_handler_class_002.php',
    '/ext/session/tests/session_set_save_handler_class_016.php',
    '/ext/session/tests/session_set_save_handler_iface_001.php',

    # unsure why
    '/ext/standard/tests/file/symlink_link_linkinfo_is_link_variation4.php',

    # don't use the internet in tests...
    '/ext/standard/tests/network/gethostbyname_error004.php',
    '/ext/standard/tests/network/getmxrr.php'

    # php coderz are so l33t
    '/ext/sockets/tests/ipv6loop.php',
    '/ext/sockets/tests/socket_getpeername_ipv4loop.php',
    '/ext/standard/tests/network/fsockopen_variation2.php',
    '/ext/sockets/tests/socket_create_listen.php',
    '/ext/sockets/tests/socket_create_listen-win32.php',

    # it references a whole directory with *
    '/ext/standard/tests/file/copy_variation6.php',

    # Tests for a bug in PHP
    '/ext/gmp/tests/014.php',

    # Another process can be created in the meantime
    '/ext/posix/tests/posix_errno_variation2.php',

    # duplicate of a test in test/slow
    '/ext/gmp/tests/002.php',

    # Something could be on sending on that UDP port
    '/ext/standard/tests/network/bug20134.php',

    # These pass with a proper proxy setup, which our build machines don't seem
    # to have. i.e., the internet is attempted to be used and that is bad
    '/ext/soap/tests/bugs/bug40609.php',
    '/ext/soap/tests/schema/schema060.php',
    '/ext/soap/tests/schema/schema084.php',
    '/ext/soap/tests/schema/schema008.php',
    '/ext/soap/tests/schema/schema070.php',
    '/ext/soap/tests/schema/schema076.php',
    '/ext/soap/tests/schema/schema034.php',
    '/ext/soap/tests/schema/schema027.php',
    '/ext/soap/tests/schema/schema037.php',
    '/ext/soap/tests/schema/schema028.php',
    '/ext/soap/tests/schema/schema080.php',
    '/ext/soap/tests/schema/schema033.php',
    '/ext/soap/tests/schema/schema031.php',
    '/ext/soap/tests/schema/schema075.php',
    '/ext/soap/tests/schema/schema015.php',
    '/ext/soap/tests/schema/schema018.php',
    '/ext/soap/tests/schema/schema069.php',
    '/ext/soap/tests/schema/schema065.php',
    '/ext/soap/tests/schema/schema019.php',
    '/ext/soap/tests/schema/schema061.php',
    '/ext/soap/tests/schema/schema074.php',
    '/ext/soap/tests/schema/schema071.php',
    '/ext/soap/tests/schema/schema077.php',
    '/ext/soap/tests/schema/schema017.php',
    '/ext/soap/tests/schema/schema005.php',
    '/ext/soap/tests/schema/schema058.php',
    '/ext/soap/tests/schema/schema003.php',
    '/ext/soap/tests/schema/schema079.php',
    '/ext/soap/tests/schema/schema032.php',
    '/ext/soap/tests/schema/schema047.php',
    '/ext/soap/tests/schema/schema004.php',
    '/ext/soap/tests/schema/schema016.php',
    '/ext/soap/tests/schema/schema045.php',
    '/ext/soap/tests/schema/schema039.php',
    '/ext/soap/tests/schema/schema026.php',
    '/ext/soap/tests/schema/schema038.php',
    '/ext/soap/tests/schema/schema001.php',
    '/ext/soap/tests/schema/schema050.php',
    '/ext/soap/tests/schema/schema041.php',
    '/ext/soap/tests/schema/schema083.php',
    '/ext/soap/tests/schema/schema011.php',
    '/ext/soap/tests/schema/schema062.php',
    '/ext/soap/tests/schema/schema029.php',
    '/ext/soap/tests/schema/schema073.php',
    '/ext/soap/tests/schema/schema025.php',
    '/ext/soap/tests/schema/schema044.php',
    '/ext/soap/tests/schema/schema023.php',
    '/ext/soap/tests/schema/schema014.php',
    '/ext/soap/tests/schema/schema052.php',
    '/ext/soap/tests/schema/schema024.php',
    '/ext/soap/tests/schema/schema072.php',
    '/ext/soap/tests/schema/schema006.php',
    '/ext/soap/tests/schema/schema082.php',
    '/ext/soap/tests/schema/schema053.php',
    '/ext/soap/tests/schema/schema085.php',
    '/ext/soap/tests/schema/schema049.php',
    '/ext/soap/tests/schema/schema063.php',
    '/ext/soap/tests/schema/schema040.php',
    '/ext/soap/tests/schema/schema043.php',
    '/ext/soap/tests/schema/schema066.php',
    '/ext/soap/tests/schema/schema048.php',
    '/ext/soap/tests/schema/schema046.php',
    '/ext/soap/tests/schema/schema007.php',
    '/ext/soap/tests/schema/schema056.php',
    '/ext/soap/tests/schema/schema067.php',
    '/ext/soap/tests/schema/schema042.php',
    '/ext/soap/tests/schema/schema059.php',
    '/ext/soap/tests/schema/schema054.php',
    '/ext/soap/tests/schema/schema036.php',
    '/ext/soap/tests/schema/schema057.php',
    '/ext/soap/tests/schema/schema002.php',
    '/ext/soap/tests/schema/schema013.php',
    '/ext/soap/tests/schema/schema051.php',
    '/ext/soap/tests/schema/schema009.php',
    '/ext/soap/tests/schema/schema078.php',
    '/ext/soap/tests/schema/schema081.php',
    '/ext/soap/tests/schema/schema030.php',
    '/ext/soap/tests/schema/schema055.php',
    '/ext/soap/tests/schema/schema021.php',
    '/ext/soap/tests/schema/schema035.php',
    '/ext/standard/tests/network/http-stream.php',

    # Socket based test that are possibly port clowny
    # Github commit: 6ae44fc92acb63e7fa31b5fd4fcd4cf939c3ef54
    '/ext/sockets/tests/socket_getsockname.php',

    # Rely on system service configuration being a particular way.
    '/ext/standard/tests/general_functions/getservbyname_variation9.php',
    '/ext/standard/tests/general_functions/getservbyname_variation10.php',
    '/ext/standard/tests/general_functions/getservbyport_variation1.php',
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
    '/Zend/tests/bug34064.php',
    '/Zend/tests/bug36006.php',
    '/Zend/tests/bug36759.php',
    '/Zend/tests/bug39542.php',
    '/Zend/tests/bug43651.php',
    '/Zend/tests/bug44141.php',
    '/Zend/tests/bug47593.php',
    '/Zend/tests/bug55007.php',
    '/Zend/tests/bug55578.php',
    '/Zend/tests/bug60771.php',
    '/Zend/tests/bug63741.php',
    '/Zend/tests/bug67436/bug67436.php',
    '/Zend/tests/class_alias_013.php',
    '/Zend/tests/class_constants_003.php',
    '/Zend/tests/class_exists_001.php',
    '/Zend/tests/closure_040.php',
    '/Zend/tests/closure_042.php',
    '/Zend/tests/constants_005.php',
    '/Zend/tests/errmsg_007.php',
    '/Zend/tests/errmsg_026.php',
    '/Zend/tests/errmsg_035.php',
    '/Zend/tests/errmsg_036.php',
    '/Zend/tests/error_reporting03.php',
    '/Zend/tests/error_reporting04.php',
    '/Zend/tests/error_reporting08.php',
    '/Zend/tests/error_reporting09.php',
    '/Zend/tests/halt_compiler2.php',
    '/Zend/tests/jump14.php',
    '/Zend/tests/lsb_013.php',
    '/Zend/tests/lsb_021.php',
    '/Zend/tests/lsb_022.php',
    '/Zend/tests/ns_041.php',
    '/Zend/tests/objects_029.php',
    '/Zend/tests/objects_030.php',
    '/Zend/tests/traits/bug55554b.php',
    '/Zend/tests/traits/bug60369.php',
    '/Zend/tests/traits/bug60809.php',
    '/Zend/tests/traits/bugs/overridding-conflicting-property-initializer.php',
    '/Zend/tests/traits/error_003.php',
    '/Zend/tests/traits/property003.php',
    '/Zend/tests/traits/property004.php',
    '/Zend/tests/unset_cv01.php',
    '/ext/bz2/tests/with_strings.php',
    '/ext/pcre/tests/preg_replace.php',
    '/ext/pdo_mysql/tests/pdo_mysql_connect_charset.php',
    '/ext/pdo_sqlite/tests/bug33841.php',
    '/ext/pdo_sqlite/tests/bug46139.php',
    '/ext/pdo_sqlite/tests/bug52487.php',
    '/ext/phar/tests/012.php',
    # missing undefined variable notice
    '/ext/posix/tests/posix_kill_variation1.php',
    '/ext/posix/tests/posix_kill_variation2.php',
    '/ext/posix/tests/posix_strerror_variation1.php',
    '/ext/posix/tests/posix_getgrgid_variation.php',
    '/ext/posix/tests/posix_getpwuid_variation.php',
    ####################################
    '/ext/reflection/tests/bug64936.php',
    '/ext/reflection/tests/bug29268.php',
    '/ext/reflection/tests/traits005.php',
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
    '/ext/sqlite3/tests/sqlite3_22_loadextension.php',
    '/ext/sqlite3/tests/sqlite3_23_escape_string.php',
    '/ext/sqlite3/tests/sqlite3_24_last_insert_rowid.php',
    '/ext/sqlite3/tests/sqlite3stmt_paramCount_basic.php',
    '/ext/sqlite3/tests/sqlite3stmt_paramCount_error.php',
    '/ext/standard/tests/array/001.php',
    '/ext/standard/tests/array/003.php',
    '/ext/standard/tests/array/sizeof_variation4.php',
    '/ext/standard/tests/assert/assert.php',
    '/ext/standard/tests/class_object/class_exists_basic_001.php',
    '/ext/standard/tests/class_object/get_declared_classes_variation1.php',
    '/ext/standard/tests/class_object/get_declared_interfaces_variation1.php',
    '/ext/standard/tests/class_object/get_declared_traits_variation1.php',
    '/ext/standard/tests/class_object/get_class_methods_variation_001',
    '/ext/standard/tests/class_object/interface_exists_variation3.php',
    '/ext/standard/tests/class_object/interface_exists_variation4.php',
    '/ext/standard/tests/class_object/is_a_variation_001.php',
    '/ext/standard/tests/class_object/is_subclass_of_variation_002',
    '/ext/standard/tests/class_object/method_exists_variation_002',
    '/ext/standard/tests/class_object/property_exists_variation1',
    '/ext/standard/tests/file/file_get_contents_basic.php',
    '/ext/standard/tests/file/file_get_contents_file_put_contents_basic.php',
    '/ext/standard/tests/file/file_get_contents_file_put_contents_variation1.php',
    '/ext/standard/tests/file/file_get_contents_file_put_contents_variation2.php',
    '/ext/standard/tests/file/file_get_contents_variation1.php',
    '/ext/standard/tests/file/readfile_variation6.php',
    '/ext/standard/tests/file/unlink_variation8.php',
    '/ext/standard/tests/file/unlink_variation10.php',
    '/ext/standard/tests/general_functions/is_callable_error.php',
    '/ext/standard/tests/general_functions/is_numeric.php',
    '/ext/standard/tests/math/abs.php',
    '/ext/standard/tests/math/acos_basic.php',
    '/ext/standard/tests/math/acosh_basic.php',
    '/ext/standard/tests/math/asin_basic.php',
    '/ext/standard/tests/math/asinh_basic.php',
    '/ext/standard/tests/math/atan_basic.php',
    '/ext/standard/tests/math/atanh_basic.php',
    '/ext/standard/tests/math/cos_basic.php',
    '/ext/standard/tests/math/cosh_basic.php',
    '/ext/standard/tests/math/deg2rad_basic.php',
    '/ext/standard/tests/math/log10_basic.php',
    '/ext/standard/tests/math/pow.php',
    '/ext/standard/tests/math/rad2deg_basic.php',
    '/ext/standard/tests/math/sin_basic.php',
    '/ext/standard/tests/math/sinh_basic.php',
    '/ext/standard/tests/math/tan_basic.php',
    '/ext/standard/tests/math/tanh_basic.php',
    '/ext/standard/tests/serialize/bug30234.php',
    '/ext/standard/tests/streams/bug64770.php',
    '/ext/standard/tests/streams/stream_resolve_include_path.php',
    '/ext/standard/tests/strings/bug44242.php',
    '/ext/standard/tests/strings/trim.php',
    '/ext/standard/tests/strings/wordwrap.php',
    '/ext/standard/tests/url/base64_encode_variation_001.php',
    '/ext/standard/tests/url/parse_url_basic_001.php',
    '/ext/standard/tests/url/parse_url_basic_002.php',
    '/ext/standard/tests/url/parse_url_basic_003.php',
    '/ext/standard/tests/url/parse_url_basic_004.php',
    '/ext/standard/tests/url/parse_url_basic_005.php',
    '/ext/standard/tests/url/parse_url_basic_006.php',
    '/ext/standard/tests/url/parse_url_basic_007.php',
    '/ext/standard/tests/url/parse_url_basic_008.php',
    '/ext/standard/tests/url/parse_url_basic_009.php',
    '/ext/standard/tests/url/parse_url_variation_001.php',
    '/ext/standard/tests/url/parse_url_variation_002_64bit.php',
    '/ext/standard/tests/url/rawurldecode_variation_001.php',
    '/ext/standard/tests/url/rawurlencode_variation_001.php',
    '/ext/standard/tests/url/urldecode_variation_001.php',
    '/ext/standard/tests/url/urlencode_variation_001.php',
    '/ext/tokenizer/tests/token_get_all_variation19.php',
    '/ext/xsl/tests/bug48221.php',
    '/ext/xsl/tests/bug54446.php',
    '/ext/xsl/tests/xslt001.php',
    '/ext/xsl/tests/xslt002.php',
    '/ext/xsl/tests/xslt003.php',
    '/ext/xsl/tests/xslt004.php',
    '/ext/xsl/tests/xslt005.php',
    '/ext/xsl/tests/xslt006.php',
    '/ext/xsl/tests/xslt007.php',
    '/ext/xsl/tests/xsltprocessor_getParameter-invalidparam.php',
    '/ext/xsl/tests/xsltprocessor_getParameter.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-allfuncs.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-array-multiple.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-array-notallowed.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-array.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-funcnostring.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-funcundef.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-null.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-string-multiple.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-string-notallowed.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-string.php',
    '/ext/xsl/tests/xsltprocessor_removeParameter-invalidparam.php',
    '/ext/xsl/tests/xsltprocessor_removeParameter.php',
    '/ext/xsl/tests/xsltprocessor_setparameter-errorquote.php',
    '/ext/xsl/tests/xsltprocessor_setparameter-nostring.php',
    '/ext/zip/tests/bug53579.php',
    '/ext/zip/tests/bug64342_1.php',
    '/ext/zip/tests/bug7658.php',
    '/ext/zip/tests/oo_addemptydir.php',
    '/ext/zip/tests/oo_addfile.php',
    '/ext/zip/tests/oo_extract.php',
    '/ext/zip/tests/oo_getcomment.php',
    '/ext/zip/tests/oo_getnameindex.php',
    '/ext/zip/tests/oo_namelocate.php',
    '/ext/zip/tests/oo_rename.php',
    '/ext/zip/tests/oo_setcomment.php',
    '/ext/zip/tests/oo_stream.php',
    '/ext/zlib/tests/gzcompress_basic1.php',
    '/ext/zlib/tests/gzcompress_variation1.php',
    '/ext/zlib/tests/gzdeflate_basic1.php',
    '/ext/zlib/tests/gzdeflate_variation1.php',
    '/ext/zlib/tests/gzencode_basic1.php',
    '/ext/zlib/tests/gzencode_variation1-win32.php',
    '/ext/zlib/tests/gzencode_variation1.php',
    '/ext/zlib/tests/gzinflate_error1.php',
    '/ext/zlib/tests/gzuncompress_basic1.php',
    '/tests/classes/autoload_001.php',
    '/tests/classes/autoload_002.php',
    '/tests/classes/autoload_003.php',
    '/tests/classes/autoload_004.php',
    '/tests/classes/autoload_005.php',
    '/tests/classes/autoload_006.php',
    '/tests/classes/autoload_010.php',
    '/tests/classes/autoload_018.php',
    '/tests/classes/constants_scope_001.php',
    '/tests/classes/unset_properties.php',
    '/tests/lang/019.php',
    '/tests/lang/034.php',
    '/tests/lang/bug25922.php',
    '/tests/lang/bug32924.php',
    '/tests/lang/include_variation3.php',
    '/tests/lang/static_variation_001.php',
    '/tests/lang/static_variation_002.php',

    # This test passes "by accident".
    'ext/dom/tests/DOMNode_hasChildNodes.php',

    # These tests use eval(), which is banned in repo mode.
    '/Zend/tests/bug31102.php',
    '/Zend/tests/bug33116.php',
    '/Zend/tests/bug36513.php',
    '/Zend/tests/bug43128.php',
    '/Zend/tests/bug47714.php',
    '/Zend/tests/bug54624.php',
    '/Zend/tests/bug60444.php',
    '/Zend/tests/bug62907.php',
    '/Zend/tests/bug63305.php',
    '/Zend/tests/bug65254.php',
    '/Zend/tests/constants/dir-constant-eval.php',
    '/Zend/tests/generators/bug67497.php',
    '/Zend/tests/is_a.php',
    '/Zend/tests/method_static_var.php',
    '/ext/session/tests/bug53141.php',
    '/ext/spl/tests/spl_autoload_014.php',
    '/ext/spl/tests/spl_autoload_bug48541.php',
    '/ext/standard/tests/class_object/is_a.php',
    '/ext/standard/tests/general_functions/bug35229.php',
    '/ext/standard/tests/serialize/bug62836_1.php',
    '/tests/classes/constants_basic_006.php',
    '/tests/lang/013.php',
    '/tests/lang/014.php',
    '/tests/lang/018.php',
    '/tests/lang/bug21961.php',
    '/tests/lang/foreachLoop.012.php',

    # XSL: 'include "prepare.inc"' makes repo mode fail.
    '/ext/xsl/tests/bug48221.php',
    '/ext/xsl/tests/bug54446.php',
    '/ext/xsl/tests/bug54446_with_ini.php',
    '/ext/xsl/tests/xslt001.php',
    '/ext/xsl/tests/xslt002.php',
    '/ext/xsl/tests/xslt003.php',
    '/ext/xsl/tests/xslt004.php',
    '/ext/xsl/tests/xslt005.php',
    '/ext/xsl/tests/xslt006.php',
    '/ext/xsl/tests/xslt007.php',
    '/ext/xsl/tests/xslt008.php',
    '/ext/xsl/tests/xslt009.php',
    '/ext/xsl/tests/xsltprocessor_getParameter-invalidparam.php',
    '/ext/xsl/tests/xsltprocessor_getParameter.php',
    '/ext/xsl/tests/xsltprocessor_getParameter-wrongparam.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-allfuncs.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-array-multiple.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-array-notallowed.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-array.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-funcnostring.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-funcundef.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-null.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-string-multiple.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-string-notallowed.php',
    '/ext/xsl/tests/xsltprocessor_registerPHPFunctions-string.php',
    '/ext/xsl/tests/xsltprocessor_removeParameter-invalidparam.php',
    '/ext/xsl/tests/xsltprocessor_removeParameter.php',
    '/ext/xsl/tests/xsltprocessor_removeParameter-wrongparams.php',
    '/ext/xsl/tests/xsltprocessor_setparameter-errorquote.php',
    '/ext/xsl/tests/xsltprocessor_setparameter-nostring.php',

    # These tests use create_function, which is basically eval.
    '/Zend/tests/anonymous_func_001.php',
    '/Zend/tests/anonymous_func_002.php',
    '/Zend/tests/anonymous_func_003.php',
    '/Zend/tests/closure_025.php',
    '/Zend/tests/instanceof_001.php',
    '/ext/spl/tests/bug61697.php',
    '/ext/standard/tests/array/array_filter_variation7.php',
    '/ext/standard/tests/array/array_map_variation10.php',
    '/ext/standard/tests/array/array_walk_recursive_variation7.php',
    '/ext/standard/tests/array/array_walk_variation7.php',
    '/ext/standard/tests/array/uasort_variation7.php',
    '/ext/standard/tests/array/usort_variation7.php',
    '/ext/standard/tests/strings/bug37262.php',
    '/tests/lang/bug17115.php',
    '/tests/lang/bug22690.php',
    '/tests/lang/bug24926.php',

    # These tests use assert with a string argument, which is also basically
    # eval.
    '/ext/dom/tests/DOMNode_insertBefore_error2.php',
    '/ext/dom/tests/DOMNode_insertBefore_error3.php',
    '/ext/dom/tests/DOMNode_insertBefore_error4.php',
    '/ext/dom/tests/DOMNode_insertBefore_error5.php',
    '/ext/dom/tests/DOMNode_insertBefore_error6.php',
    '/tests/lang/bug23922.php',

    # This creates an interface with the same name as a builtin, which
    # hphpc doesn't correctly support AttrUnique flags on.
    '/Zend/tests/inter_06.php',

    # Tests use banned reflection features
    '/ext/reflection/tests/bug30146.php',

    # Closure::bind.
    '/Zend/tests/anon/013.php',
)

# Random other files that zend wants
other_files = (
    '/ext/bz2/tests/004_1.txt.bz2',
    '/ext/bz2/tests/004_2.txt.bz2',
    '/ext/calendar/tests/skipif.inc',
    '/ext/ctype/tests/skipif.inc',
    '/ext/curl/tests/curl_testdata1.txt',
    '/ext/curl/tests/curl_testdata2.txt',
    '/ext/curl/tests/responder/get.php',
    '/ext/curl/tests/server.inc',
    '/ext/curl/tests/skipif.inc',
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
    '/ext/dba/tests/skipif.inc',
    '/ext/dom/tests/book.xml',
    '/ext/dom/tests/book.xml.gz',
    '/ext/dom/tests/book.xsd',
    '/ext/dom/tests/book-attr.xml',
    '/ext/dom/tests/book-non-conforming-schema.xsd',
    '/ext/dom/tests/book-not-a-schema.xsd',
    '/ext/dom/tests/bug67081_0.xml',
    '/ext/dom/tests/bug67081_1.xml',
    '/ext/dom/tests/bug67081_2.xml',
    '/ext/dom/tests/dom_test.inc',
    '/ext/dom/tests/dom.xml',
    '/ext/dom/tests/dom.ent',
    '/ext/dom/tests/empty.html',
    '/ext/dom/tests/note.xml',
    '/ext/dom/tests/not_well.html',
    '/ext/dom/tests/nsdoc.xml',
    '/ext/dom/tests/skipif.inc',
    '/ext/dom/tests/test.html',
    '/ext/dom/tests/xinclude.xml',
    '/ext/exif/tests/bug34704.jpg',
    '/ext/exif/tests/bug48378.jpeg',
    '/ext/exif/tests/bug60150.jpg',
    '/ext/exif/tests/bug62523_1.jpg',
    '/ext/exif/tests/bug62523_2.jpg',
    '/ext/exif/tests/bug62523_3.jpg',
    '/ext/exif/tests/exif_encoding_crash.jpg',
    '/ext/exif/tests/image007.jpg',
    '/ext/exif/tests/image008.jpg',
    '/ext/exif/tests/image009.jpg',
    '/ext/exif/tests/image010.jpg',
    '/ext/exif/tests/image011.jpg',
    '/ext/exif/tests/image012.jpg',
    '/ext/exif/tests/image013.jpg',
    '/ext/exif/tests/image014.jpg',
    '/ext/exif/tests/image015.jpg',
    '/ext/exif/tests/image016.tiff',
    '/ext/exif/tests/image017.tiff',
    '/ext/exif/tests/image018.tiff',
    '/ext/exif/tests/image020.tiff',
    '/ext/exif/tests/image021.tiff',
    '/ext/exif/tests/image022.tiff',
    '/ext/exif/tests/image023.tiff',
    '/ext/exif/tests/image024.jpg',
    '/ext/exif/tests/image025.jpg',
    '/ext/exif/tests/image026.tiff',
    '/ext/exif/tests/image027.tiff',
    '/ext/exif/tests/test1.jpg',
    '/ext/exif/tests/test2.jpg',
    '/ext/exif/tests/test5.jpg',
    '/ext/fileinfo/tests/magic',
    '/ext/fileinfo/tests/resources/dir.zip',
    '/ext/fileinfo/tests/resources/test.awk',
    '/ext/fileinfo/tests/resources/test.bmp',
    '/ext/fileinfo/tests/resources/test.gif',
    '/ext/fileinfo/tests/resources/test.jpg',
    '/ext/fileinfo/tests/resources/test.mp3',
    '/ext/fileinfo/tests/resources/test.pdf',
    '/ext/fileinfo/tests/resources/test.png',
    '/ext/fileinfo/tests/resources/test.ppt',
    '/ext/fileinfo/tests/skipif.inc',
    '/ext/ftp/tests/cert.pem',
    '/ext/ftp/tests/server.inc',
    '/ext/ftp/tests/skipif.inc',
    '/ext/gd/tests/bug37346.gif',
    '/ext/gd/tests/bug38112.gif',
    '/ext/gd/tests/bug43121.gif',
    '/ext/gd/tests/conv_test.gif',
    '/ext/gd/tests/conv_test.jpeg',
    '/ext/gd/tests/conv_test.png',
    '/ext/gd/tests/conv_test.xbm',
    '/ext/gd/tests/crafted.gd2',
    '/ext/gd/tests/php.gif',
    '/ext/gd/tests/Rochester-Regular.otf',
    '/ext/gd/tests/Rochester-Regular.otf.LICENSE.txt',
    '/ext/gd/tests/src.gd2',
    '/ext/gd/tests/src.wbmp',
    '/ext/gd/tests/test8859.ttf',
    '/ext/gd/tests/test.png',
    '/ext/gd/tests/Tuffy.ttf',
    '/ext/gd/tests/logo_noise.png',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest.mo',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest.po',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest_switched.po',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest_switch.mo',
    '/ext/gettext/tests/locale/en/LC_CTYPE/dgettextTest_switch.po',
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
    '/ext/iconv/tests/skipif.inc',
    '/ext/imap/tests/skipif.inc',
    '/ext/interbase/tests/skipif.inc',
    '/ext/intl/tests/ut_common.inc',
    '/ext/ldap/tests/connect.inc',
    '/ext/ldap/tests/skipifbindfailure.inc',
    '/ext/ldap/tests/skipif.inc',
    '/ext/mbstring/tests/common.inc',
    '/ext/mbstring/tests/skipif.inc',
    '/ext/mcrypt/tests/vectors.txt',
    '/ext/mysqli/tests/clean_table.inc',
    '/ext/mysqli/tests/connect.inc',
    '/ext/mysqli/tests/skipifconnectfailure.inc',
    '/ext/mysqli/tests/skipifemb.inc',
    '/ext/mysqli/tests/skipif.inc',
    '/ext/mysqli/tests/skipifnotemb.inc',
    '/ext/mysqli/tests/skipifunicode.inc',
    '/ext/mysqli/tests/table.inc',
    '/ext/mysql/tests/connect.inc',
    '/ext/mysql/tests/skipif.inc',
    '/ext/mysql/tests/table.inc',
    '/ext/oci8/tests/skipif.inc',
    '/ext/odbc/tests/skipif.inc',
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
    '/ext/openssl/tests/ServerClientTestCase.inc',
    '/ext/pdo_firebird/tests/skipif.inc',
    '/ext/pdo_mysql/tests/common.phpt',
    '/ext/pdo_mysql/tests/config.inc',
    '/ext/pdo_mysql/tests/skipif.inc',
    '/ext/pdo_sqlite/tests/common.phpt',
    '/ext/pdo/tests/pdo_test.inc',
    '/ext/phar/tests/files/phar_test.inc',
    '/ext/phar/tests/files/stuboflength1041.phar',
    '/ext/phar/tests/tar/files/P1-1.0.0.tgz',
    '/ext/reflection/tests/bug64936.inc',
    '/ext/reflection/tests/included4.inc',
    '/ext/session/tests/save_handler.inc',
    '/ext/session/tests/skipif.inc',
    '/ext/simplexml/tests/book.xml',
    '/ext/simplexml/tests/bug24392.xml',
    '/ext/snmp/tests/skipif.inc',
    '/ext/soap/tests/bugs/bug27722.wsdl',
    '/ext/soap/tests/bugs/bug28985.wsdl',
    '/ext/soap/tests/bugs/bug29109.wsdl',
    '/ext/soap/tests/bugs/bug29236.wsdl',
    '/ext/soap/tests/bugs/bug29795.wsdl',
    '/ext/soap/tests/bugs/bug30106.wsdl',
    '/ext/soap/tests/bugs/bug30175.wsdl',
    '/ext/soap/tests/bugs/bug30928.wsdl',
    '/ext/soap/tests/bugs/bug34643.wsdl',
    '/ext/soap/tests/bugs/bug36614.wsdl',
    '/ext/soap/tests/bugs/bug36908.wsdl',
    '/ext/soap/tests/bugs/bug36999.wsdl',
    '/ext/soap/tests/bugs/bug37013.wsdl',
    '/ext/soap/tests/bugs/bug37083.wsdl',
    '/ext/soap/tests/bugs/bug38004.wsdl',
    '/ext/soap/tests/bugs/bug38055.wsdl',
    '/ext/soap/tests/bugs/bug38067.wsdl',
    '/ext/soap/tests/bugs/bug38536.wsdl',
    '/ext/soap/tests/bugs/bug40609.wsdl',
    '/ext/soap/tests/bugs/bug40609.phpt',
    '/ext/soap/tests/bugs/bug41337.wsdl',
    '/ext/soap/tests/bugs/bug42326.wsdl',
    '/ext/soap/tests/bugs/bug42692.wsdl',
    '/ext/soap/tests/bugs/skipif.inc',
    '/ext/soap/tests/classmap.wsdl',
    '/ext/soap/tests/server030.wsdl',
    '/ext/soap/tests/interop/Round2/Base/skipif.inc',
    '/ext/soap/tests/interop/Round2/GroupB/skipif.inc',
    '/ext/soap/tests/interop/Round3/GroupD/skipif.inc',
    '/ext/soap/tests/interop/Round3/GroupE/skipif.inc',
    '/ext/soap/tests/interop/Round3/GroupF/round3_groupF_extreq.wsdl',
    '/ext/soap/tests/interop/Round3/GroupF/skipif.inc',
    '/ext/soap/tests/interop/Round4/GroupG/skipif.inc',
    '/ext/soap/tests/interop/Round4/GroupH/skipif.inc',
    '/ext/soap/tests/interop/Round4/GroupI/skipif.inc',
    '/ext/soap/tests/schema/skipif.inc',
    '/ext/soap/tests/schema/test_schema.inc',
    '/ext/soap/tests/server025.wsdl',
    '/ext/soap/tests/skipif.inc',
    '/ext/soap/tests/soap12/skipif.inc',
    '/ext/soap/tests/soap12/soap12-test.inc',
    '/ext/soap/tests/soap12/soap12-test.wsdl',
    '/ext/spl/tests/fileobject_001a.txt',
    '/ext/spl/tests/fileobject_001b.txt',
    '/ext/spl/tests/SplFileObject_testinput.csv',
    '/ext/spl/tests/testclass.class.inc',
    '/ext/sqlite3/tests/new_db.inc',
    '/ext/sqlite3/tests/skipif.inc',
    '/ext/sqlite3/tests/stream_test.inc',
    '/ext/standard/tests/array/compare_function.inc',
    '/ext/standard/tests/array/data.inc',
    '/ext/standard/tests/class_object/AutoInterface.inc',
    '/ext/standard/tests/class_object/AutoLoaded.inc',
    '/ext/standard/tests/class_object/AutoTest.inc',
    '/ext/standard/tests/class_object/AutoTrait.inc',
    '/ext/standard/tests/file/bug30362.txt',
    '/ext/standard/tests/file/bug38086.txt',
    '/ext/standard/tests/file/bug40501.csv',
    '/ext/standard/tests/file/file.inc',
    '/ext/standard/tests/file/fopen_include_path.inc',
    '/ext/standard/tests/file/stream_rfc2397_003.gif',
    '/ext/standard/tests/file/test2.csv',
    '/ext/standard/tests/file/test3.csv',
    '/ext/standard/tests/file/test.csv',
    '/ext/standard/tests/general_functions/004.data',
    '/ext/standard/tests/general_functions/bug49692.ini',
    '/ext/standard/tests/general_functions/bug52138.data',
    '/ext/standard/tests/general_functions/get_included_files_inc1.inc',
    '/ext/standard/tests/general_functions/get_included_files_inc2.inc',
    '/ext/standard/tests/general_functions/get_included_files_inc3.inc',
    '/ext/standard/tests/general_functions/parse_ini_basic.data',
    '/ext/standard/tests/general_functions/parse_ini_booleans.data',
    '/ext/standard/tests/image/200x100.bmp',
    '/ext/standard/tests/image/200x100.gif',
    '/ext/standard/tests/image/200x100.jpg',
    '/ext/standard/tests/image/200x100.png',
    '/ext/standard/tests/image/200x100.swf',
    '/ext/standard/tests/image/200x100.tif',
    '/ext/standard/tests/image/200x100_unknown.unknown',
    '/ext/standard/tests/image/246x247.png',
    '/ext/standard/tests/image/2x2mm.tif',
    '/ext/standard/tests/image/384x385.png',
    '/ext/standard/tests/image/75x50.wbmp',
    '/ext/standard/tests/image/75x50.xbm',
    '/ext/standard/tests/image/blank_file.bmp',
    '/ext/standard/tests/image/bug13213.jpg',
    '/ext/standard/tests/image/skipif_imagetype.inc',
    '/ext/standard/tests/image/test13pix.swf',
    '/ext/standard/tests/image/test1bpix.bmp',
    '/ext/standard/tests/image/test-1pix.bmp',
    '/ext/standard/tests/image/test1pix.bmp',
    '/ext/standard/tests/image/test1pix.jp2',
    '/ext/standard/tests/image/test1pix.jpc',
    '/ext/standard/tests/image/test1pix.jpg',
    '/ext/standard/tests/image/test2pix.gif',
    '/ext/standard/tests/image/test4pix.gif',
    '/ext/standard/tests/image/test4pix.iff',
    '/ext/standard/tests/image/test4pix.png',
    '/ext/standard/tests/image/test4pix.psd',
    '/ext/standard/tests/image/test4pix.swf',
    '/ext/standard/tests/image/test4pix.tif',
    '/ext/standard/tests/image/testAPP.jpg',
    '/ext/standard/tests/image/test.gif',
    '/ext/standard/tests/image/test.txt',
    '/ext/standard/tests/math/allowed_rounding_error.inc',
    '/ext/standard/tests/serialize/autoload_implements.p5c',
    '/ext/standard/tests/serialize/autoload_interface.p5c',
    '/ext/standard/tests/url/urls.inc',
    '/ext/sybase_ct/tests/skipif.inc',
    '/ext/xmlreader/tests/012.dtd',
    '/ext/xmlreader/tests/012.xml',
    '/ext/xmlreader/tests/dtdexample.dtd',
    '/ext/xmlreader/tests/relaxNG2.rng',
    '/ext/xmlreader/tests/relaxNG3.rng',
    '/ext/xmlreader/tests/relaxNG.rng',
    '/ext/xml/tests/skipif.inc',
    '/ext/xml/tests/xmltest.xml',
    '/ext/xsl/tests/53965/collection.xml',
    '/ext/xsl/tests/53965/collection.xsl',
    '/ext/xsl/tests/53965/include.xsl',
    '/ext/xsl/tests/area_list.xsl',
    '/ext/xsl/tests/area_name.xml',
    '/ext/xsl/tests/bug49634.xml',
    '/ext/xsl/tests/documentxpath.xsl',
    '/ext/xsl/tests/exslt.xml',
    '/ext/xsl/tests/exslt.xsl',
    '/ext/xsl/tests/phpfunc-nostring.xsl',
    '/ext/xsl/tests/phpfunc-undef.xsl',
    '/ext/xsl/tests/phpfunc.xsl',
    '/ext/xsl/tests/prepare.inc',
    '/ext/xsl/tests/skipif.inc',
    '/ext/xsl/tests/streamsinclude.xsl',
    '/ext/xsl/tests/xslt011.xml',
    '/ext/xsl/tests/xslt011.xsl',
    '/ext/xsl/tests/xslt012.xsl',
    '/ext/xsl/tests/xslt.xml',
    '/ext/xsl/tests/xslt.xsl',
    '/ext/xsl/tests/xslt.xsl.gz',
    '/ext/zip/tests/utils.inc',
    '/ext/zip/tests/test_with_comment.zip',
    '/ext/zlib/tests/004.txt.gz',
    '/ext/zlib/tests/bug_52944_corrupted_data.inc',
    '/ext/zlib/tests/data.inc',
    '/ext/zlib/tests/gzopen_include_path.inc',
    '/ext/zlib/tests/reading_include_path.inc',
    '/tests/classes/autoload_derived.p5c',
    '/tests/classes/autoload_implements.p5c',
    '/tests/classes/autoload_interface.p5c',
    '/tests/classes/autoload_root.p5c',
    '/tests/classes/constants_basic_003.inc',
    '/tests/classes/interface_optional_arg_003.inc',
    '/tests/lang/015.inc',
    '/tests/lang/016.inc',
    '/tests/lang/023-1.inc',
    '/tests/lang/023-2.inc',
    '/tests/lang/inc.inc',
    '/tests/lang/include_files/echo.inc',
    '/tests/lang/include_files/eval.inc',
    '/tests/lang/include_files/function.inc',
    '/tests/lang/inc_throw.inc',
    '/tests/quicktester.inc',
    '/Zend/tests/014.inc',
    '/Zend/tests/bug39542/bug39542.php',
    '/Zend/tests/bug46665_autoload.inc',
    '/Zend/tests/bug54804.inc',
    '/Zend/tests/bug67436/a.php',
    '/Zend/tests/bug67436/b.php',
    '/Zend/tests/bug67436/c.php',
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
    '/Zend/tests/use_const/includes/foo_bar.php',
    '/Zend/tests/use_const/includes/foo_php_version.php',
    '/Zend/tests/use_const/includes/global_bar.php',
    '/Zend/tests/use_const/includes/global_baz.php',
    '/Zend/tests/use_function/includes/foo_bar.php',
    '/Zend/tests/use_function/includes/foo_strlen.php',
    '/Zend/tests/use_function/includes/global_bar.php',
    '/Zend/tests/use_function/includes/global_baz.php',
)

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        pass

def walk(filename, dest_subdir):
    dest_filename = os.path.basename(filename)

    mkdir_p(dest_subdir)
    full_dest_filename = os.path.join(dest_subdir, dest_filename)

    # Exactly mirror zend's directories incase some tests depend on random crap.
    # We'll only move things we want into 'good'. The condition below is in case
    # of using --local on a file already in the current workind directory.
    if os.path.abspath(filename) != os.path.abspath(full_dest_filename):
        shutil.copyfile(filename, full_dest_filename)

    full_dest_filename = full_dest_filename.replace('.phpt', '.php')

    if not '.phpt' in filename:
        data = open(full_dest_filename).read()

        if '/ext/mysqli/tests/table.inc' in full_dest_filename:
            data = data.replace(
                'DROP TABLE IF EXISTS test\'',
                'DROP TABLE IF EXISTS \'.$test_table_name'
            )
            data = data.replace(
                'CREATE TABLE test',
                'CREATE TABLE \'.$test_table_name.\'',
            )
            data = data.replace(
                'INSERT INTO test',
                'INSERT INTO ".$test_table_name."',
            )

        if '/ext/mysqli/tests/clean_table.inc' in full_dest_filename:
            data = data.replace(
                'DROP TABLE IF EXISTS test\'',
                'DROP TABLE IF EXISTS \'.$test_table_name'
            )

        if '/tests/security/open_basedir.inc' in full_dest_filename:
            data = data.replace(
                'getcwd()',
                '__DIR__',
            )

        open(full_dest_filename, 'w').write(data)

        if full_dest_filename.endswith('.php'):
            f = open(full_dest_filename.replace('.php', '.php.skipif'), 'w')
            f.write('always skip - not a test')

        return

    print("Importing %s" % full_dest_filename)

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

    sections = parse_headers(open(filename).read())
    for i in list(sections.keys()):
        sections[i] = '\n'.join(sections[i])

    unsupported_sections = ('POST_RAW')
    for name in unsupported_sections:
        if name in sections:
            print("Unsupported test with section --%s--: " % name, filename)
            return

    if not 'FILE' in sections:
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
                    '\nFatal error: '+match_rest_of_line, exp)
            exp = re.sub(r'(?:Catchable fatal)\\? error\\?:.*',
                    '\nCatchable fatal error: '+match_rest_of_line, exp)
            exp = re.sub(r'Warning\\?:.*',
                    '\nWarning: '+match_rest_of_line, exp)
            exp = re.sub(r'Notice\\?:.*',
                    '\nNotice: '+match_rest_of_line, exp)
            exp = re.sub(r'object\((\w+)\)#\d+', 'object(\\1)#%d', exp)

            exp = re.sub('string\(7\) "Closure"', 'string(%d) "Closure%s"', exp)
            exp = re.sub('Closure(?!%s)', 'Closure%s', exp)

            sections[key] = exp

    if 'EXPECT' in sections:
        exp = sections['EXPECT']

        if '/ext/zip/tests/bug51353.php' in full_dest_filename:
            exp = exp.replace('100000', '1000')

        # we use %a for error messages so always write expectf
        open(full_dest_filename + '.expectf', 'w').write(exp)
    elif 'EXPECTREGEX' in sections:
        exp = sections['EXPECTREGEX']

        if '/ext/standard/tests/strings/004.php' in full_dest_filename:
            exp = exp.replace(': 3', ': [3-4]')

        open(full_dest_filename + '.expectregex', 'w').write(exp)
    elif 'EXPECTF' in sections:
        exp = sections['EXPECTF']

        if ('/ext/standard/tests/url/parse_url_variation_002_64bit.php' in full_dest_filename or
          '/ext/zlib/tests/gzfile_variation13.php' in full_dest_filename or
          '/ext/zlib/tests/gzopen_variation3.php' in full_dest_filename or
          '/ext/zlib/tests/readgzfile_variation13.php' in full_dest_filename):
            exp = exp.replace('to be long', 'to be integer')

        open(full_dest_filename + '.expectf', 'w').write(exp)
    else:
        print("Malformed test, no --EXPECT*--: ", filename)
        return

    if 'INI' in sections:
        ini = sections['INI']

        if '/tests/lang/bug24908.php' in full_dest_filename or \
           '/ext/spl/tests/spl_autoload_011.php' in full_dest_filename:
            ini = ini + '\n' + 'hhvm.enable_obj_destruct_call = true'
        if '/ext/calendar/tests/cal_info.php' in full_dest_filename or \
           '/ext/calendar/tests/easter_date.php' in full_dest_filename or \
           '/ext/calendar/tests/jdtounix.php' in full_dest_filename or \
           '/ext/date/tests/cal_days_in_month_invalid_date.php' in full_dest_filename or \
           '/ext/date/tests/cal_days_in_month_invalid_calendar.php' in full_dest_filename or \
           '/ext/soap/tests/bugs/bug36999.php' in full_dest_filename:
            ini = ini + '\n' + 'hhvm.enable_zend_compat = true'

        open(full_dest_filename + '.ini', 'w').write(ini)

    if 'SKIPIF' in sections:
        skipif = sections['SKIPIF'].strip()
        if skipif[:2] != '<?':
            skipif = '<?php' + skipif

        if '/ext/standard/tests/strings/fprintf_' in full_dest_filename:
            skipif = skipif.replace('dump.txt', dest_filename + '.txt')
        # php-src#817
        if ('/ext/zlib/tests/gzfile_variation4.php' in full_dest_filename or
            '/ext/zlib/tests/readgzfile_variation4.php' in full_dest_filename):
            skipif = skipif.replace("extension_loaded(zlib)",
                                    "extension_loaded('zlib')")

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
            '<?php\n$_COOKIE = http_parse_cookie("' +
                sections['COOKIE'] + '");\n'
            '_filter_snapshot_globals();\n'
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

    # If you put an exception in here, please send a pull request upstream to
    # php-src. Then when it gets merged kill your hack.
    if '/tests/classes/bug63462.php' in full_dest_filename:
        exp = exp.replace("Notice:", "\nNotice:")
        open(full_dest_filename + '.expectf', 'w').write(exp)
    if ('/ext/ldap/tests/ldap_control_paged_results_variation1.php' in full_dest_filename) or \
       ('extldap/tests/ldap_control_paged_results_variation2.php' in full_dest_filename):
        exp = exp.replace("resource(6)", "resource(%d)")
        open(full_dest_filename + '.expectf', 'w').write(exp)
    if ('/ext/standard/tests/math/pow.php' in full_dest_filename) or \
       ('/ext/standard/tests/math/abs.php' in full_dest_filename):
        # HHVM handles int->double promotion differently than Zend
        # But the pow() function behaves the same, so we have to fudge it a bit
        test = test.replace("9223372036854775807",
                            "(double)9223372036854775807")
        test = test.replace("0x7FFFFFFF", "(double)0x7FFFFFFF")
        test = test.replace("is_int(LONG_MIN  )", "is_float(LONG_MIN  )")
        test = test.replace("is_int(LONG_MAX  )", "is_float(LONG_MAX  )")
    if '/ext/standard/tests/strings/vfprintf_' in full_dest_filename:
        test = test.replace('dump.txt', dest_filename + '.txt')
        test = test.replace('vfprintf_test.txt', dest_filename + '.txt')
    if '/ext/standard/tests/strings/fprintf_' in full_dest_filename:
        test = test.replace('dump.txt', dest_filename + '.txt')
    if '/ext/standard/tests/file/filesize_variation3-win32.php' in full_dest_filename:
        test = test.replace('filesize_variation3', 'filesize_variation3-win32')
    if '/ext/spl/tests/bug42364.php' in full_dest_filename:
        test = test.replace('dirname(__FILE__)', '__DIR__."/../../../../../sample_dir/"')
    if '/ext/standard/tests/file/bug24482.php' in full_dest_filename:
        test = test.replace('"*"', '__DIR__."/../../../../../../sample_dir/*"')
        test = test.replace('opendir(".")', 'opendir(__DIR__."/../../../../../../sample_dir/")')
        test = test.replace('is_dir($file)', 'is_dir(__DIR__."/../../../../../../sample_dir/".$file)')
    if '/ext/standard/tests/file/touch_variation1.php' in full_dest_filename:
        test = test.replace('touch.dat', 'touch_variation1.dat')
    if '/ext/standard/tests/file/bug45181.php' in full_dest_filename:
        test = test.replace('chdir("bug45181_x");', '$origdir = getcwd();\nchdir("bug45181_x");')
        test = test.replace('rmdir("bug45181_x");', 'rmdir($origdir . "/bug45181_x");')
    if '/ext/standard/tests/file/fgets_socket_variation1.php' in full_dest_filename:
        test = test.replace("<?php", "<?php\n$port = rand(50000, 65535);")
        test = test.replace("31337'", "'.$port")
    if '/ext/standard/tests/file/fputcsv.php' in full_dest_filename:
        test = test.replace("fgetcsv.csv", "fputcsv.csv")
    if '/ext/spl/tests/SplFileObject_fgetcsv_' in full_dest_filename:
        test = test.replace('SplFileObject__fgetcsv.csv',
            os.path.basename(full_dest_filename).replace('.php', '.csv'))
    if '/ext/spl/tests/SplFileObject_fputcsv_' in full_dest_filename:
        test = test.replace('SplFileObject_fputcsv.csv',
            os.path.basename(full_dest_filename).replace('.php', '.csv'))
    if '/ext/zip/tests/oo_getnameindex.php' in full_dest_filename:
        test = test.replace('__tmp_oo_rename.zip', '__tmp_oo_rename2.zip')
        test = test.replace('var_dump($zip->getNameIndex(3));',
                            'var_dump($zip->getNameIndex(3));\n@unlink($file);')
    if '/ext/zip/tests/oo_namelocate.php' in full_dest_filename:
        test = test.replace('__tmp_oo_rename.zip', '__tmp_oo_rename3.zip')
        test = test.replace('?>', '@unlink($file);\n?>')
    if '/ext/zip/tests/oo_addemptydir.php' in full_dest_filename:
        test = test.replace('__tmp_oo_addfile.zip', '__tmp_oo_addfile2.zip')
    if '/ext/zip/tests/oo_addfile.php' in full_dest_filename:
        test = test.replace('__tmp_oo_addfile.zip', '__tmp_oo_addfile3.zip')
    if '/ext/zip/tests/bug51353.php' in full_dest_filename:
        test = test.replace('100000', '1000')
    if '/ext/zip/tests/bug8700.php' in full_dest_filename:
        test = test.replace('content_from', 'contents_from')
        test = test.replace(
            'if ($contents_from_idx != $contents_from_name)',
            'if ($contents_from_idx != "=)" || $contents_from_name != "=)")'
            )
    if '/Zend/tests/bug36759.php' in full_dest_filename:
        pseudomain = '$y = new Bar();\n$x = new Foo($y);\n'
        test = test.replace(pseudomain,
                            'function main() {\n' + pseudomain + '}\nmain();\n')
    if '/Zend/tests/bug55007.php' in full_dest_filename:
        # Fixed in php-src@0ec49bba (probably PHP7)
        test = test.replace('$a[]', '$a[];')
    if '/ext/phar/tests/' in full_dest_filename:
        test = test.replace('.clean', '')
    if '/ext/phar/tests/' in full_dest_filename:
        test = test.replace('.clean', '')
    if ('/ext/readline/tests/readline_info_001.php' in full_dest_filename) or \
       ('/ext/readline/tests/libedit_info_001.php' in full_dest_filename):
        test = test.replace('readline_info(', '@readline_info(')
    if '/ext/standard/tests/file/file_get_contents_variation1.php' in full_dest_filename:
        test = test.replace('afile.txt', 'file_get_contents_variation1.txt')
    if '/ext/standard/tests/file/readfile_variation6.php' in full_dest_filename:
        test = test.replace('"readfileVar6.dir"', "tempnam(sys_get_temp_dir(), 'rfv6');\nunlink($thisTestDir)")
        test = test.replace('afile.txt', 'readfile_variation6.txt')
    if '/ext/standard/tests/file/rename_variation' in full_dest_filename:
        test = test.replace('rename_variation_dir', dest_filename+'_dir')
        test = test.replace('rename_variation.tmp', dest_filename+'.tmp')
        test = test.replace('rename_variation2.tmp', dest_filename+'2.tmp')
        test = test.replace('rename_variation_link.tmp', dest_filename+'_link.tmp')
    if '/ext/standard/tests/file/bug46347.php' in full_dest_filename:
        test = test.replace('/parse.ini', '/bug46347.ini')
    if '/ext/standard/tests/file/parse_ini_file.php' in full_dest_filename:
        test = test.replace('/parse.ini', '/parse_ini_file.ini')
    if '/ext/standard/tests/strings/sha1_file.php' in full_dest_filename:
        test = test.replace('DataFile.txt', 'sha1_DataFile.txt')
        test = test.replace('EmptyFile.txt', 'sha1_EmptyFile.txt')
    if '/ext/standard/tests/strings/md5_file.php' in full_dest_filename:
        test = test.replace('DataFile.txt', 'md5_DataFile.txt')
        test = test.replace('EmptyFile.txt', 'md5_EmptyFile.txt')
    if '/ext/intl/tests/calendar_getNow_basic.php' in full_dest_filename:
        test = test.replace('500)', '1000)')
        test = test.replace('1000)', '2000)')
    if '/Zend/tests/closure_016.php' in full_dest_filename:
        # undo closure% only for first two instances
        exp = exp.replace('Closure%s::', 'Closure::', 2)
        open(full_dest_filename + '.expectf', 'w').write(exp)
    if '/ext/json/tests/unsupported_type_error.php' in full_dest_filename:
        exp = exp.replace('resource(5)', 'resource(%d)')
        open(full_dest_filename + '.expectf', 'w').write(exp)
    if ('/ext/xmlreader/tests/007.php' in full_dest_filename or
       '/ext/xmlreader/tests/008.php' in full_dest_filename or
       '/ext/xmlreader/tests/012.php' in full_dest_filename or
       '/ext/dom/tests/bug37456.php' in full_dest_filename or
       '/ext/dom/tests/bug67081.php' in full_dest_filename or
       '/ext/xsl/tests/bug53965.php' in full_dest_filename):
        open(full_dest_filename + '.ini', 'w').write(
                'hhvm.libxml.ext_entity_whitelist = "file"')
    if '/ext/xsl/tests/xslt008.php' in full_dest_filename:
        open(full_dest_filename + '.ini', 'w').write(
                'hhvm.libxml.ext_entity_whitelist = "compress.zlib"')
    if '/ext/standard/tests/file/realpath_basic' in full_dest_filename:
        root_dest_filenname = os.path.splitext(dest_filename)[0]
        test = test.replace('realpath_basic', root_dest_filenname)
        exp = exp.replace('realpath_basic', root_dest_filenname)
        open(full_dest_filename + '.expectf', 'w').write(exp)
    if '/ext/standard/tests/network/bug20134.php' in full_dest_filename:
        test = test.replace("<?php", "<?php\n$port = rand(1025, 65535);")
        test = test.replace("65534", "$port")
    if '/ext/dom/tests/DOMDocument_saveHTMLFile_basic.php' in full_dest_filename:
        test = test.replace('tmp_savehtmlfile', 'DOMDocument_saveHTMLFile_basic')
    if '/ext/dom/tests/DOMDocument_saveHTMLFile_formatOutput.php' in full_dest_filename:
        test = test.replace('tmp_savehtmlfile', 'DOMDocument_saveHTMLFile_formatOutput')
    if '/ext/standard/tests/file/fscanf_variation53.php' in full_dest_filename:
        test = test.replace('fscanf_variation52.tmp', 'fscanf_variation53.tmp')
    if '/ext/mysqli/tests/' in full_dest_filename:

        (testname, _) = os.path.splitext(os.path.basename(full_dest_filename))

        replace_configs = {
            'table': {
                '002': ['test_fetch_null'],
                '003': ['test_bind_result'],
                '004': ['test_bind_fetch'],
                '005': ['test_bind_fetch'],
                '006': ['test_bind_fetch'],
                '007': ['test_bind_fetch'],
                '008': ['test_bind_fetch'],
                '009': ['test_bind_fetch'],
                '010': ['test_bind_fetch'],
                '011': ['test_bind_result'],
                '012': ['test_bind_result'],
                '013': ['test_bind_result'],
                '014': ['test'],
                '015': ['test'],
                '019': ['insert_read'],
                '020': ['test_bind_result'],
                '021': ['test_bind_fetch'],
                '022': ['test_bind_fetch'],
                '023': ['test_bind_fetch'],
                '024': ['test_bind_fetch'],
                '025': ['test_bind_fetch'],
                '026': ['test_bind_fetch'],
                '029': ['general_test'],
                '030': ['non_exisiting_table'],
                '031': ['non_exisiting_table'],
                '032': ['general_test'],
                '036': ['t036'],
                '037': ['test_result'],
                '038': ['test_result'],
                '040': ['test_result'],
                '041': ['test_warnings'],
                '042': ['test_bind_fetch'],
                '043': ['test_update'],
                '046': ['test_affected'],
                '047': ['test_affected'],
                '048': ['test_fetch_null'],
                '057': ['test_store_result'],
                '058': ['mbind'],
                '059': ['mbind'],
                '060': ['test_fetch'],
                '061': ['t_061'],
                '062': ['DUAL'],
                '063': ['DUAL'],
                '064': ['DUAL'],
                '066': ['test_warnings'],
                '067': [re.compile('cursor(?=(%d|\$i))')],
                'bug32405': ['test_users'],
                'bug34810': ['test_warnings'],
                'bug34785': ['DUAL'],
                'bug35103': ['test_bint', 'test_buint'],
                'bug35517': ['temp'],
                'bug35759': ['test'],
                'bug36745': ['litest'],
                'bug36802': ['DUAL'],
                'bug36949': ['DUAL'],
                'bug42378': [re.compile('test(?!_format)')],
                'bug44897': ['test'],
                'bug45289': ['test'],
                'bug48909': ['test'],
                'bug49027': ['test'],
                'bug49442': ['test'],
                'bug52891': ['tuint', 'tsint'],
                'bug53503': ['test'],
                'bug54221': ['t54221'],
                'mysqli_affected_rows': ['test'],
                'mysqli_affected_rows_oo': ['test'],
                'mysqli_autocommit':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_autocommit_oo':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_change_user_insert_id': ['test'],
                'mysqli_change_user_locks_temporary':
                    [re.compile('(?<= )test')],
                'mysqli_change_user_rollback': ['test'],
                'mysqli_character_set': [re.compile('test(?!!)')],
                'mysqli_class_mysqli_properties_no_conn': ['test'],
                'mysqli_class_mysqli_result_interface': ['test'],
                'mysqli_class_mysqli_stmt_interface': ['test'],
                'mysqli_commit':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_commit_oo':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_data_seek': ['test'],
                'mysqli_data_seek_oo': ['test'],
                'mysqli_errno':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_errno_oo':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_error':
                    [re.compile('test(?! )')],
                'mysqli_error_oo':
                    [re.compile('test(?! )')],
                'mysqli_expire_password': [re.compile('(?<= )test')],
                'mysqli_explain_metadata': ['test'],
                'mysqli_fetch_all': ['test'],
                'mysqli_fetch_all_oo': ['test'],
                'mysqli_fetch_array': ['test'],
                'mysqli_fetch_array_assoc': ['test'],
                'mysqli_fetch_array_large':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_fetch_array_many_rows': ['test'],
                'mysqli_fetch_array_oo': ['test'],
                'mysqli_fetch_assoc': [re.compile('(?<= )test(?= )')],
                'mysqli_fetch_assoc_bit': [re.compile('(?<= )test')],
                'mysqli_fetch_assoc_oo': [re.compile('(?<= )test(?= )')],
                'mysqli_fetch_assoc_zerofill': ['test'],
                'mysqli_fetch_field_direct': ['test'],
                'mysqli_fetch_field_direct_oo': ['test'],
                'mysqli_fetch_field_flags': ['test'],
                'mysqli_fetch_field': [re.compile('(?<!type )test')],
                'mysqli_fetch_field_oo': [re.compile('(?<= )test(?= )')],
                'mysqli_fetch_field_types': [re.compile('test(?!\w)')],
                'mysqli_fetch_fields': [re.compile('(?<= )test(?= )')],
                'mysqli_fetch_lengths': ['test'],
                'mysqli_fetch_lengths_oo': ['test'],
                'mysqli_fetch_object': [re.compile('(?<= )test(?= )')],
                'mysqli_fetch_object_no_constructor':
                    [re.compile('(?<= )test(?= )')],
                'mysqli_fetch_object_no_object': ['test'],
                'mysqli_fetch_object_oo': [re.compile('(?<= )test(?= )')],
                'mysqli_fetch_row': ['test'],
                'mysqli_field_count': ['test'],
                'mysqli_field_seek': ['test'],
                'mysqli_field_tell': ['test'],
                'mysqli_fork': [re.compile('test(?!s)'), 'messages'],
                'mysqli_free_result': ['test'],
                'mysqli_get_client_stats': [re.compile('(?<= )test')],
                'mysqli_get_client_stats_skipped': ['test'],
                'mysqli_info': ['test'],
                'mysqli_insert_id': ['test'],
                'mysqli_insert_packet_overflow': ['test'],
                'mysqli_kill': [re.compile('test(?= )')],
                'mysqli_last_insert_id': ['test', 'DUAL'],
                'mysqli_max_links': ['test'],
                'mysqli_more_results': ['test'],
                'mysqli_multi_query': ['test'],
                'mysqli_next_result': [re.compile('(?<= )test')],
                'mysqli_num_fields': [re.compile('(?<= )test')],
                'mysqli_num_rows': [re.compile('(?<= )test')],
                'mysqli_options_init_command': [re.compile('test(?! more)')],
                'mysqli_pconn_kill': ['test'],
                'mysqli_poll_mixing_insert_select':
                    [re.compile('test(?! may)'), 'bogus'],
                'mysqli_prepare':
                    [re.compile('(?<=(ISTS|FROM) )test(?!2)'), 'test2'],
                'mysqli_query': ['test'],
                'mysqli_query_iterators': ['test'],
                'mysqli_query_stored_proc': ['test'],
                'mysqli_real_escape_string_big5': ['test'],
                'mysqli_real_escape_string_eucjpms': ['test'],
                'mysqli_real_escape_string_euckr': ['test'],
                'mysqli_real_escape_string_gb2312': ['test'],
                'mysqli_real_escape_string_gbk': ['test'],
                'mysqli_real_escape_string_nobackslash': ['test'],
                'mysqli_real_escape_string_sjis': ['test'],
                'mysqli_real_query': ['test'],
                'mysqli_report': [re.compile('(?<=(INTO|FROM) )test')],
                'mysqli_result_references': ['test'],
                'mysqli_result_references_mysqlnd': ['test'],
                'mysqli_rollback':
                    [re.compile('(?<=(INTO|FROM|ISTS|ABLE) )test')],
                'mysqli_select_db': ['test'],
                'mysqli_sqlstate': ['test'],
                'mysqli_stmt_affected_rows':
                    [re.compile('(?<=(INTO|FROM|ISTS|ABLE) )test')],
                'mysqli_stmt_attr_get': ['test'],
                'mysqli_stmt_attr_set': [re.compile('(?<=FROM )test')],
                'mysqli_stmt_bind_limits': ['test'],
                'mysqli_stmt_bind_param':
                    [re.compile('(?<=(INTO|FROM|ISTS|ABLE) )test')],
                'mysqli_stmt_bind_param_call_user_func': ['test'],
                'mysqli_stmt_bind_param_references': ['test'],
                'mysqli_stmt_bind_param_type_juggling':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_stmt_bind_result': [re.compile('test(?!(s| is))')],
                'mysqli_stmt_bind_result_bit': [re.compile('test(?!s)')],
                'mysqli_stmt_bind_result_format':
                    [re.compile('test(?!_format)'), 'DUAL'],
                'mysqli_stmt_bind_result_references': ['test'],
                'mysqli_stmt_bind_result_zerofill': [re.compile('test(?= )')],
                'mysqli_stmt_close': ['test'],
                'mysqli_stmt_data_seek': ['test'],
                'mysqli_stmt_errno': ['test'],
                'mysqli_stmt_error': ['test'],
                'mysqli_stmt_execute': [re.compile('(?<=(INTO|FROM) )test')],
                'mysqli_stmt_fetch': [re.compile('(?<=FROM )test')],
                'mysqli_stmt_fetch_bit': [re.compile('test(?!s)')],
                'mysqli_stmt_fetch_fields_win32_unicode': ['test'],
                'mysqli_stmt_fetch_geom': ['test'],
                'mysqli_stmt_field_count': ['test'],
                'mysqli_stmt_free_result': [re.compile('(?<=FROM )test')],
                'mysqli_stmt_get_result': [re.compile('(?<=FROM )test')],
                'mysqli_stmt_get_result_bit': [re.compile('(?<= )test')],
                'mysqli_stmt_get_result_field_count': ['test'],
                'mysqli_stmt_get_result_geom': ['test'],
                'mysqli_stmt_get_result_metadata':
                    [re.compile('(?<=FROM )test')],
                'mysqli_stmt_get_result_metadata_fetch_field': ['test'],
                'mysqli_stmt_get_result_seek': ['test'],
                'mysqli_stmt_get_result_types':
                    [re.compile('test(?!( is broken|s))')],
                'mysqli_stmt_get_result2': [re.compile('(?<=FROM )test')],
                'mysqli_stmt_insert_id': ['test'],
                'mysqli_stmt_num_rows': [re.compile('(?<!run_)test')],
                'mysqli_stmt_param_count': ['test'],
                'mysqli_stmt_prepare': [re.compile('(?<=FROM )test')],
                'mysqli_stmt_reset':
                    [re.compile('(?<=(INTO|ISTS|ABLE|FROM) )test')],
                'mysqli_stmt_result_metadata': ['test'],
                'mysqli_stmt_send_long_data': [re.compile('(?<!=we )test')],
                'mysqli_stmt_send_long_data_packet_size_libmysql':
                    [re.compile('(?<=(INTO|ISTS|ABLE) )test')],
                'mysqli_stmt_sqlstate': ['test'],
                'mysqli_stmt_store_result':
                    [re.compile('(?<=(INTO|FROM) )test')],
                'mysqli_store_result': ['test'],
                'mysqli_use_result': ['test'],
                'mysqli_warning_count': ['test'],
                'mysqli_warning_unclonable': ['test'],
            },
            'procedure': {
                'bug42548': ['p1'],
                'bug44897': [re.compile('p(?=[\'"(])')],
                'mysqli_poll_mixing_insert_select': [re.compile('p(?=[\'"(])')],
                'mysqli_query': [re.compile('(?<= )p(?=[\'"(])')],
                'mysqli_query_stored_proc': [re.compile('(?<= )p(?=[\'"(])')],
            },
            'function': {
                'mysqli_query': [re.compile('(?<= )f(?=[\'"(])')],
            },
            'lock': {
                'mysqli_change_user_locks_temporary': ['phptest'],
                'mysqli_prepare': ['testlock'],
            },
            'var': {
                'mysqli_prepare': ['testvar'],
            }
        }

        # This remove the ParamCoerceMode checks that is usually in the beginning
        # of the tests. Remove this when we have ParamCoerceMode working for PHP
        # methods
        r = re.compile('^\s*if \(.*@.*\)[\s\r\n]*.*Expecting.*[\r\n]+',
                       re.MULTILINE)
        test = r.sub('', test)

        for t, t_replace_config in replace_configs.items():
            for i, replace_id in enumerate(t_replace_config.get(testname, [])):
                new_id = 'test_%s_%s_%d' % (testname, t, i + 1)
                if isinstance(replace_id, str):
                    test = test.replace(replace_id, new_id)
                else:
                    test = replace_id.sub(new_id, test)

        new_id = 'test_%s_table_1' % (testname, )
        test = re.sub('(require(_once)*[ (][\'"](clean_)?table.inc[\'"]\)?)',
                      '$test_table_name = \'%s\'; \\1' % (new_id, ), test)

    open(full_dest_filename, 'w').write(test)

def should_import(filename):
    for bad in no_import:
        if bad in filename:
            return False
    return True

# -- main --

parser = argparse.ArgumentParser()
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
zend_dir = os.path.normpath(os.path.join(script_dir, '../zend'))
all_dir = os.path.join(zend_dir, 'all')

zend_release_name = "php-" + zend_version
zend_release_filename = zend_release_name + ".tar.gz"
zend_release_archive = os.path.join(zend_dir, zend_release_filename)
zend_release_path = os.path.join(zend_dir, zend_release_name)

if not os.path.isfile(zend_release_archive):
    print('Downloading ' + zend_release_name + '...')
    zend_release_url = "http://php.net/get/" + zend_release_filename + "/from/this/mirror"
    zend_release_request = urllib2.Request(zend_release_url)
    zend_release_response = urllib2.urlopen(zend_release_request)
    open(zend_release_archive, 'wb').write(zend_release_response.read())

if not os.path.isdir(zend_release_path):
    print('Extracting ' + zend_release_name + '...')
    zend_release_tar = tarfile.open(zend_release_archive, 'r:gz')
    zend_release_tar.extractall(zend_dir)

for root, dirs, files in os.walk(zend_release_path):
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

        if matches(args.only) and should_import(full_file):
            walk(
                full_file,
                os.path.join(all_dir, os.path.relpath(root, zend_release_path))
            )

if not os.path.isdir(all_dir):
    if args.only:
        print("No test/zend/all. " +
              "Your --only arg didn't match any test that should be imported.")
    else:
        print("No test/zend/all. Maybe no tests were imported?")
    sys.exit(0)
else:
    print("Running all tests from zend/all")

stdout = subprocess.Popen(
    [
        os.path.join(script_dir, '../run'),
        '--fbmake',
        '-m',
        'interp',
        all_dir,
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
    print(results)
else:
    print("Test run done, moving files")

for test in results:
    filename = test['name']

    good_file = filename.replace('all', 'good', 1)
    bad_file = filename.replace('all', 'bad', 1)
    flaky_file = filename.replace('all', 'flaky', 1)

    all_files = [good_file, bad_file, flaky_file]

    for test_file in all_files:
        mkdir_p(os.path.dirname(test_file))

    good = (test['status'] == 'passed')

    flaky_test = False
    for test in flaky_tests:
        if test in filename:
            good = False
            flaky_test = True

    needs_norepo = False
    for test in norepo_tests:
        if test in filename:
            needs_norepo = True

    if good:
        dest_file = good_file
        subpath = 'good'
    elif not flaky_test:
        dest_file = bad_file
        subpath = 'bad'
    else:
        dest_file = flaky_file
        subpath = 'flaky'

    exps = glob.glob(filename + '.expect*')
    if not exps:
        # this file is probably generated while running tests :(
        continue

    source_file_exp = exps[0]
    _, dest_ext = os.path.splitext(source_file_exp)
    shutil.copyfile(filename, dest_file)
    open(dest_file + dest_ext, 'w').write(
        open(source_file_exp).read().replace('/all', '/' + subpath)
    )

    if needs_norepo:
        open(dest_file + '.norepo', 'w')
    if os.path.exists(filename + '.skipif'):
        shutil.copyfile(filename + '.skipif', dest_file + '.skipif')
    if os.path.exists(filename + '.ini'):
        shutil.copyfile(filename + '.ini', dest_file + '.ini')

    for test_file in all_files:
        if dest_file == test_file:
            continue

        for f in glob.glob(test_file + "*"):
            if os.path.isfile(f):
                os.unlink(f)
            else:
                shutil.rmtree(f)

# extra random files needed for tests...
for root, dirs, files in os.walk(all_dir):
    for filename in files:
        filename = os.path.join(root, filename)

        for name in other_files:
            if name in filename:
                for subdir in ('good', 'bad'):
                    dest = filename.replace('all', subdir, 1)
                    dir = os.path.dirname(dest)
                    mkdir_p(dir)
                    data = open(filename).read()

                    if '/ext/ftp/tests/server.inc' in filename:
                        data = data.replace('<10', '<100')
                        data = data.replace('50000', '1025')
                        data = data.replace('stream_socket_server', '@stream_socket_server')

                    open(dest, 'w').write(data)

if not args.dirty:
    shutil.rmtree(all_dir)
