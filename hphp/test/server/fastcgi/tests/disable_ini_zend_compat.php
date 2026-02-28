<?hh

// Remove this test once we unify the zend ini compat diff (D1797805) with the
// per dir diff (D2099778)

function disableIniZendCompatController($port) :mixed{
  echo request('localhost', $port, 'test_disable_ini_zend_compat.php');
}
<<__EntryPoint>> function main(): void {
require_once('test_base.inc');
init();
runTest(disableIniZendCompatController<>);
}
