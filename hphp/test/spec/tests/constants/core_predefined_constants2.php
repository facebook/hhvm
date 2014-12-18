<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function trace($text, $pdc)
{
    echo "$text: ";
    var_dump($pdc);
}

trace("PHP_VERSION", PHP_VERSION);
trace("PHP_MAJOR_VERSION", PHP_MAJOR_VERSION);
trace("PHP_MINOR_VERSION", PHP_MINOR_VERSION);
trace("PHP_RELEASE_VERSION", PHP_RELEASE_VERSION);
trace("PHP_VERSION_ID", PHP_VERSION_ID);
trace("PHP_EXTRA_VERSION", PHP_EXTRA_VERSION);
// HHVM doesn't have this
// trace("PHP_ZTS", PHP_ZTS);
trace("PHP_DEBUG", PHP_DEBUG);
trace("PHP_MAXPATHLEN", PHP_MAXPATHLEN);
trace("PHP_OS", PHP_OS);
trace("PHP_SAPI", PHP_SAPI);
trace("PHP_EOL", PHP_EOL);
trace("DEFAULT_INCLUDE_PATH", DEFAULT_INCLUDE_PATH);
trace("PEAR_INSTALL_DIR", PEAR_INSTALL_DIR);
trace("PEAR_EXTENSION_DIR", PEAR_EXTENSION_DIR);
trace("PHP_EXTENSION_DIR", PHP_EXTENSION_DIR);
trace("PHP_PREFIX", PHP_PREFIX);
trace("PHP_BINDIR", PHP_BINDIR);
// HHVM doesn't have this
// trace("PHP_MANDIR", PHP_MANDIR);
trace("PHP_LIBDIR", PHP_LIBDIR);
trace("PHP_DATADIR", PHP_DATADIR);
trace("PHP_SYSCONFDIR", PHP_SYSCONFDIR);
trace("PHP_CONFIG_FILE_PATH", PHP_CONFIG_FILE_PATH);
trace("PHP_CONFIG_FILE_SCAN_DIR", PHP_CONFIG_FILE_SCAN_DIR);
trace("PHP_SHLIB_SUFFIX", PHP_SHLIB_SUFFIX);
trace("E_ERROR", E_ERROR);
trace("E_WARNING", E_WARNING);
trace("E_PARSE", E_PARSE);
trace("E_NOTICE", E_NOTICE);
trace("E_CORE_ERROR", E_CORE_ERROR);
trace("E_CORE_WARNING", E_CORE_WARNING);
trace("E_COMPILE_ERROR", E_COMPILE_ERROR);
trace("E_COMPILE_WARNING", E_COMPILE_WARNING);
trace("E_USER_ERROR", E_USER_ERROR);
trace("E_USER_WARNING", E_USER_WARNING);
trace("E_USER_NOTICE", E_USER_NOTICE);
trace("E_DEPRECATED", E_DEPRECATED);
trace("E_USER_DEPRECATED", E_USER_DEPRECATED);
trace("E_ALL", E_ALL);
trace("E_STRICT", E_STRICT);
trace("__COMPILER_HALT_OFFSET__", __COMPILER_HALT_OFFSET__);
trace("TRUE", TRUE);
trace("FALSE", FALSE);
trace("NULL", NULL);
__HALT_COMPILER();
