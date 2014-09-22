<?php

var_dump(defined('HHVM_VERSION'));
var_dump(defined('HHVM_VERSION_ID'));
var_dump(is_int(HHVM_VERSION_ID));

// test HHVM_VERSION and HHVM_VERSION_ID matches
$version = explode('.', HHVM_VERSION);
$version_int = $version[0] * 10000 + $version[1] * 100 + $version[2];
if (HHVM_VERSION_ID == $version_int) {
  echo "HHVM_VERSION matches HHVM_VERSION_ID\n";
} else {
  echo "HHVM_VERSION does NOT match HHVM_VERSION_ID, please make sure update both of them\n";
}
