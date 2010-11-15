<?php

chdir(preg_replace('#/bin/license.php$#', '/src', realpath(__FILE__)));

// parse all these files
$inputs = 'find . -regex ".*\.cpp" -or -regex ".*\.c"   -or -regex ".*\.h"';

// do not touch these files
$excluded = array(
  'runtime/ext/bcmath/',
  'util/neo/',
  'util/cronoutils.h',
  'util/cronoutils.cpp',
  'runtime/ext/quicklz.h',
  'runtime/ext/quicklz.inc',
  'runtime/ext/ext_memcached.h',
  'runtime/ext/ext_memcached.cpp',
  'test/test_ext_memcached.h',
  'test/test_ext_memcached.cpp',

  // non-PHP licenses
  'runtime/base/zend/utf8_decode.c',
  'runtime/base/zend/utf8_to_utf16.c',
  'runtime/base/zend/zend_strtod.cpp',
  'runtime/ext/JSON_parser.cpp',
  'runtime/ext/php_unicode.h',
  'runtime/base/zend/zend_ini.tab.cpp',
  'third_party/',
);

$files_zend = array(
  'runtime/base/zend/',
  'runtime/base/array/zend_array.cpp',
);
$files_php = array(
  'runtime/ext/',
  'runtime/base/server/upload.h',
  'runtime/base/server/upload.cpp',
  'util/compression.cpp',
);

$generated_files = array(
  'system/gen/',
  'lex.yy.cpp',
  'lex.eval_.cpp',
  'hphp.tab.cpp',
);

$license_zend = <<<LICENSE
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/
LICENSE;

$license_hiphop = <<<LICENSE
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
LICENSE;

$license_php = <<<LICENSE
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
LICENSE;


///////////////////////////////////////////////////////////////////////////////

$generated = '@' . 'generated'; // split so diff tool would pick me up
$files = array();
exec($inputs, $files);

foreach ($files as $file) {
  // excluding some files
  foreach ($excluded as $e) {
    if (preg_match('/'.preg_quote($e, '/').'/', $file)) {
      continue 2;
    }
  }

  $contents = file_get_contents($file);
  if ($contents === false) {
    exit("unable to read $file\n");
  }

  $pattern = '/^[\n\s]*\/\*.*?(Copyright|PHP license).*?\*\/\n(\/\/ ';
  $pattern .= $generated;
  $pattern .= '[^\n]*\n)*/s';

  // remove existing license
  while (true) {
    $replaced = preg_replace($pattern, '', $contents);
    if (!$replaced || $replaced === $contents) {
      break;
    }
    file_put_contents($file, $replaced);
    $contents = $replaced;
  }

  // add zend licese
  foreach ($files_zend as $f) {
    if (preg_match('/'.preg_quote($f, '/').'/', $file)) {
      file_put_contents($file, $license_zend."\n".$contents);
      continue 2;
    }
  }

  // add php licese
  foreach ($files_php as $f) {
    if (preg_match('/'.preg_quote($f, '/').'/', $file)) {
      file_put_contents($file, $license_php."\n".$contents);
      continue 2;
    }
  }

  // add the at-sign 'generated' flag for code review tools
  foreach ($generated_files as $f) {
    if (preg_match('/'.preg_quote($f, '/').'/', $file)) {
      $contents = "// $generated by HipHop Compiler\n".$contents;
    }
  }

  // add hiphop licese
  file_put_contents($file, $license_hiphop."\n".$contents);
}
