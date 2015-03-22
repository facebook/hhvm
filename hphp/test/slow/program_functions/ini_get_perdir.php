<?php
require_once __DIR__ . '/ini_get_filtered_impl.inc';

run_tests(function($v)  {
  //
  // alas, PHP_INI_PERDIR isn't exposed as a PHP constant,
  // even though bitsets built from the various PHP_INI_FOO
  // values are exposed through the "access" slot.
  //
  // See ./hphp/runtime/base/ini-setting.h
  // But note the wonky encoding of PHP_INI_ALL==(1<<4) and how it
  // diverges from Zend's encoding PHP_INI_ALL==7
  //
  $PHP_INI_PERDIR = (1 << 1);
  return $v["access"] === $PHP_INI_PERDIR;
});

