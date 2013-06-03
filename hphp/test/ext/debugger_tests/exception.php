<?php
require_once("hphpd.php");

error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("exception", "debugger_tests");
if (!$client) {
  echo FAIL;
  return;
}

function testFin($c) {
  // clear and continue
  $o = $c->processCmd('break', array('clear', 'all'));
  VS($o['values'], null);
  $c->processCmd('continue', null);
}

function exception1($c) {
  // Built-in
  $o = $c->processCmd('exception', array('Exception'));
  VS($o['values'][0]['is_exception'], true);
  VS($o['values'][0]['exception_class'], 'Exception');
  $o = $c->processCmd('@', array('throw_exception()'));
  VS($o['output_type'], 'code_loc');
  VS(substr($o['file'],-15), 'exception_t.php');
  VS($o['line_no'], 8);
  testFin($c);
}

function exception2($c) {
  // User defined
  $o = $c->processCmd('exception', array('MyException'));
  VS($o['values'][0]['is_exception'], true);
  VS($o['values'][0]['exception_class'], 'MyException');
  $o = $c->processCmd('@', array('throw_myexception()'));
  VS($o['output_type'], 'code_loc');
  VS(substr($o['file'],-15), 'exception_t.php');
  VS($o['line_no'], 12);
  testFin($c);
}

function exception3($c) {
  // runtime error
  $o = $c->processCmd('exception', array('error'));
  VS($o['values'][0]['is_exception'], true);
  $o = $c->processCmd('@', array('error_undefined_class()'));
  VS($o['output_type'], 'code_loc');
  VS(substr($o['file'],-15), 'exception_t.php');
  VS($o['line_no'], 16);
  testFin($c);
}


try {
  $client->processCmd('@', array('include(\'exception_t.php\')'));
  exception1($client);
  exception2($client);
  exception3($client);
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
