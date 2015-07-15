<?php

error_reporting(E_ALL | E_STRICT | E_WARNING);

$dir = preg_replace('#/hphp/tools/license.php$#', '/hphp', realpath(__FILE__));
if (!chdir($dir)) {
  print("Unable to chdir to $dir\n");
  exit(1);
}

// parse all these files
$inputs = 'find . -name "*.cpp" -o -name "*.c" -o -name "*.h"';

// do not touch these files
$excluded = array(
  'runtime/base/compiler_id.h',
  'runtime/ext/bcmath/',
  'runtime/ext_zend_compat/',
  'runtime/ext/fileinfo/libmagic/',
  'neo/',
  'util/cronoutils.h',
  'util/cronoutils.cpp',
  'vixl/',
  'parser/hphp.tab.cpp',
  'runtime/base/ini-parser/zend-ini.tab.cpp',
  'runtime/ext/gd/libgd/',
  'runtime/ext/zlib/quicklz.h',
  'runtime/ext/zlib/quicklz.inc',
  'runtime/vm/jit/vtune/',
  'facebook',
  'submodules',

  // non-PHP licenses
  'runtime/base/zend-strtod.cpp',
  'runtime/ext/json/JSON_parser.cpp',
  'runtime/ext/mbstring/php_unicode.h',
  'runtime/base/zend-ini.tab.cpp',
  'util/safesort.h',
  'zend/crypt-blowfish.h',
  'zend/crypt-blowfish.c',
);

$files_external_party = array(
  'runtime/ext/memcached/ext_memcached.cpp'=> 'hyves',
);

$files_zend = array(
  'zend/',
  'runtime/base/intl-convert.cpp',
  'runtime/base/intl-convert.h',
  'runtime/base/zend-collator.h',
  'runtime/base/zend-collator.cpp',
  'runtime/base/zend-functions.h',
  'runtime/base/zend-functions.cpp',
  'runtime/base/zend-ini.lex.yy.cpp',
  'runtime/base/zend-math.h',
  'runtime/base/zend-math.cpp',
  'runtime/base/zend-multiply.h',
  'runtime/base/zend-pack.h',
  'runtime/base/zend-pack.cpp',
  'runtime/base/zend-php-config.h',
  'runtime/base/zend-printf.h',
  'runtime/base/zend-printf.cpp',
  'runtime/base/zend-qsort.h',
  'runtime/base/zend-qsort.cpp',
  'runtime/base/zend-rand.cpp',
  'runtime/base/zend-scanf.h',
  'runtime/base/zend-scanf.cpp',
  'runtime/base/zend-string.h',
  'runtime/base/zend-string.cpp',
  'runtime/base/zend-strtod.h',
  'runtime/base/zend-strtod.cpp',
  'runtime/base/zend-url.h',
  'runtime/base/zend-url.cpp',
);
$files_php = array(
  'runtime/ext/',
  'runtime/base/dummy-resource.h',
  'runtime/base/dummy-resource.cpp',
  'runtime/server/upload.h',
  'runtime/server/upload.cpp',
  'util/compression.cpp',
);

$external_licenses = array();
$external_licenses['hyves'] = 'Copyright (c) 2010 Hyves (http://www.hyves.nl)';

$cyr = date_create()->format('Y');

$license_zend = <<<LICENSE
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-$cyr Facebook, Inc. (http://www.facebook.com)     |
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
   | Copyright (c) 2010-$cyr Facebook, Inc. (http://www.facebook.com)     |
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
   | Copyright (c) 2010-$cyr Facebook, Inc. (http://www.facebook.com)     |
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

$license_external = <<<LICENSE
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
<external>
   | Copyright (c) 2010-$cyr Facebook, Inc. (http://www.facebook.com)     |
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

// license length
$license_line_length = 74;

$built_licenses = array();
foreach ($external_licenses as $key => $value) {
  $line = '   | ' . $value;
  $line .= str_repeat(' ', $license_line_length - strlen($line)) . '|';
  $built_licenses[$key] = str_replace('<external>', $line, $license_external);
}

///////////////////////////////////////////////////////////////////////////////

$generated = '@' . 'generated'; // split so diff tool would pick me up
$files = array();
exec($inputs, $files);

function process_file_contents($file, $contents) {
  global $files_external_party, $files_zend, $files_php, $generated_files,
    $external_licenses, $license_zend, $license_hiphop, $license_php,
    $license_external, $built_licenses, $generated;
  $pattern = '/^[\n\s]*\/\*.*?(Copyright|PHP license).*?\*\/\n(\/\/ ';
  $pattern .= $generated;
  $pattern .= '[^\n]*\n)*/s';

  // remove existing license
  while (true) {
    $replaced = @preg_replace($pattern, '', $contents);
    if (!$replaced || $replaced === $contents) {
      break;
    }
    $contents = $replaced;
  }

  foreach ($files_external_party as $f => $license) {
    if (preg_match('/'.preg_quote($f, '/').'/', $file)) {
      if (isset($built_licenses[$license])) {
        return $built_licenses[$license]."\n".$contents;
      }
    }
  }

  // add zend license
  foreach ($files_zend as $f) {
    if (preg_match('/'.preg_quote($f, '/').'/', $file)) {
      return $license_zend."\n".$contents;
    }
  }

  // add php license
  foreach ($files_php as $f) {
    if (preg_match('/'.preg_quote($f, '/').'/', $file)) {
      return $license_php."\n".$contents;
    }
  }

  // add hiphop license
  return $license_hiphop."\n".$contents;
}

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

  $new_contents = process_file_contents($file, $contents);
  if ($new_contents !== $contents) {
    echo "Updating license for $file\n";
    file_put_contents($file, $new_contents);
  }
}
