<?php

// Remove this test once we unify the zend ini compat diff (D1797805) with the
// per dir diff (D2099778)

require_once('test_base.inc');

function disableIniZendCompatController($port) {
  echo request(php_uname('n'), $port, 'test_disable_ini_zend_compat.php');
}

echo "---Enable Ini Zend Compat ON---\n";
runTest("disableIniZendCompatController",
        "-dhhvm.enable_zend_ini_compat=true");
echo "\n---Enable Ini Zend Compat OFF---\n";
runTest("disableIniZendCompatController",
        "-dhhvm.enable_zend_ini_compat=false");
