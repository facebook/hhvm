<?php

//
// This test is ostensibly the same as
//   ../../../slow/program_functions/ini_get_perdir.php
// It's inconvenient to require_once that file,
// since we need a wrapper to invoke the test in fcgi mode.
// The test is small enough that we'll just repeat the guts of it here.
//

function print_some_ini_get_all(callable $filter_fn) {
  $settings = ini_get_all();
  $trimmed_arr = array_filter($settings, $filter_fn);
  var_dump($trimmed_arr);
}

function print_some_ini_get_all_by_name($keys) {
  $settings = ini_get_all();
  $trimmed_arr = [];
  foreach ($keys as $key) {
    $trimmed_arr[$key] = $settings[$key];
  }
  var_dump($trimmed_arr);
}

function dumpInterestingIniSettings() {
  print_some_ini_get_all(function($v)  {
    //
    // Alas, PHP_INI_PERDIR isn't exposed as a PHP constant,
    // even though bitsets built from the various PHP_INI_FOO
    // values are exposed through the "access" slot.
    //
    // See ./hphp/runtime/base/ini-setting.h
    // But note HHVM's weird encoding of PHP_INI_ALL==(1<<4)
    // and how it diverges from Zend's encoding PHP_INI_ALL==7
    //
    $PHP_INI_PERDIR = (1 << 1);
    return $v["access"] === $PHP_INI_PERDIR;
  });

  print_some_ini_get_all_by_name([
    //
    // To test the efficacy of loading settings files,
    // these values should match what's in
    //   test/server/fastcgi/config/server.ini
    // 
    //
    'hhvm.log.always_log_unhandled_exceptions',
    'hhvm.server.type',
    'hhvm.repo.local.mode',
    // 'hhvm.repo.eval.mode',  // Called out in server.ini, but no longer used?
    'hhvm.repo.commit',

  ]);
}

dumpInterestingIniSettings();
