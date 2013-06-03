<?php
require_once("hphpd.php");

error_log("");
error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("basic", "debugger_tests");
if (!$client) {
  echo FAIL;
  return;
}

function eval1($c) {
  $c->processCmd('@', array('$a=123'));
  $o = $c->processCmd('print', array('$a'));
  VS($o['values']['body'], '$a');
  VS($o['values']['value'], 123);
  $c->processCmd('@', array('unset($a)'));
  $o = $c->processCmd('print', array('$a'));
  VS($o['values']['value'], null);
}

function eval2($c) {
  $c->processCmd('=', array('explode(\' \', \'    \')'));
  $o = $c->processCmd('print', array('$_'));
  VS($o['values']['value'], explode(' ', '    '));
}

function startupDoc($c) {
  $o = $c->processCmd('print', array('function_exists(\'testStartup\')'));
  VS($o['values']['value'], true);
}

try {
  eval1($client);
  eval2($client);
  startupDoc($client);

  // test quit and reconnect
  $client->processCmd('@', array('$a=123'));
  $o = $client->processCmd('quit', null);
  VS($o, true);
  $client = get_client("basic", "debugger_tests");
  if (!$client) {
    error_log("No client");
    echo FAIL;
    return;
  }
  $o = $client->processCmd('print', array('$a'));
  VS($o['values']['value'], null);

  eval1($client);
  eval2($client);

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

