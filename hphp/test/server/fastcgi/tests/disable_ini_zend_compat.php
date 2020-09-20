<?hh

// Remove this test once we unify the zend ini compat diff (D1797805) with the
// per dir diff (D2099778)

function disableIniZendCompatController($port) {
  echo request('localhost', $port, 'test_disable_ini_zend_compat.php');
}
<<__EntryPoint>> function main(): void {
require_once('test_base.inc');
init();
echo "---Enable Ini Zend Compat ON---\n";
runTest("disableIniZendCompatController",
        "-dhhvm.enable_zend_ini_compat=true");
echo "\n---Enable Ini Zend Compat OFF---\n";
runTest("disableIniZendCompatController",
        "-dhhvm.enable_zend_ini_compat=false");
}
