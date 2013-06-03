<?php
require_once("hphpd.php");

error_log("");
error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("info", "debugger_tests");
if (!$client) {
  error_log("No client!");
  echo FAIL;
  return;
}

function info1($c) {
  // Built-in
  $c->processCmd('@', null);
  $o = $c->processCmd('info', array('array_key_exists'));
  VS(trim($o['text']), '/**
 * ( excerpt from http://php.net/manual/en/function.array-key-exists.php )
 *
 * array_key_exists() returns TRUE if the given key is set in the array.
 * key can be any value possible for an array index.
 *
 * @key        mixed   Value to check.
 * @search     mixed   An array with keys to check.
 *
 * @return     bool    Returns TRUE on success or FALSE on failure.
 */
function array_key_exists($key, $search);');
  $o = $c->processCmd('info', array('stdClass'));
  VS(strpos($o['text'], "class stdClass {\n}") > 0, true);
}

function info2($c) {
  // User-defined
  $c->processCmd('@', null);
  $o = $c->processCmd('info', array('myfunc'));
  VS(strpos($o['text'], 'function myfunc($a, $b);') > 0, true);
  $o = $c->processCmd('info', array('MyClass'));
  VS(strpos($o['text'], "class MyClass {") > 0, true);
  $o = $c->processCmd('info', array('MyClass::pub'));
  VS(strpos($o['text'], 'public $pub;') > 0, true);
  $o = $c->processCmd('info', array('MyClass::pro'));
  VS(strpos($o['text'], 'protected $pro;') > 0, true);
  $o = $c->processCmd('info', array('MyClass::pri'));
  VS(strpos($o['text'], 'private $pri;') > 0, true);
}


try {
  info1($client);
  $client->processCmd('@', array('include(\'info_t.php\')'));
  info2($client);
  $o = $client->processCmd('quit', null);
  VS($o, true);
  echo PASS;
} catch (TestFailure $t) {
  error_log($t);
  echo FAIL;
} catch (Exception $e) {
  error_log($e);
  echo FAIL;
}
