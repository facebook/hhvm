<?php

//
// This is NOT a test, per-se.
// It is "called" on the server side by
//    hphp/test/server/fastcgi/tests/inivars.php
//

function probeIniVar($setting_name) {
  //
  // We don't care about the value, just its type, and for strings, the
  // length. The value may change between builds; by discarding the
  // value we make the tests more robust, but less diagnostic, across
  // builds.
  //
  $setting_value = ini_get($setting_name);
  $setting_type = gettype($setting_value);
  $setting_length = -1;
  if ($setting_type === "string") {
    $setting_length = strlen($setting_value);
  }
  printf("setting name=%-35s type=%-10s length=%-2d value=%s\n",
    $setting_name, $setting_type, $setting_length, $setting_value);
}

//
// These ini vars are the ones implemented as PHP_INI_PERDIR.
// See:
//   https://github.com/facebook/hhvm/issues/4993
// which showed how some PERDIR ini settings were not gettable
// when HHVM was run in fcgi mode.
//
function inivarsTestPerDir() {
  probeIniVar("hhvm.server.type");
  probeIniVar("hhvm.repo.local.mode");

  probeIniVar("post_max_size");
  probeIniVar("auto_prepend_file");
  probeIniVar("auto_append_file");
  probeIniVar("always_populate_raw_post_data");
  probeIniVar("upload_max_filesize");
}

inivarsTestPerDir();
