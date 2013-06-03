<?php
require_once("hphpd.php");
require_once("hphpd_test_inc.php");

error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("web_request", "debugger_tests");
if (!$client) {
  echo FAIL;
  return;
}

function signal1($c) {
  // Wait for an interrupt with in the request.
  sendToHarness('2');
  $o = $c->processCmd('continue', null);
  VS($o['output_type'], 'code_loc');
  VS(substr($o['file'],-17), 'web_request_t.php');
  VS($o['line_no'], 20);
  $o = $c->processCmd('print', array('$a'));
  VS($o['values']['value'], 1);
  // Break the endless loop in web_request_t.php.
  $c->processCmd('@', array('$a=0'));
}

function signal2($c) {
  // Stop on request and PSP end.
  $c->processCmd('break', array('end', '/web_request_t.php'));
  $c->processCmd('break', array('psp', '/web_request_t.php'));
  $o = $c->processCmd('continue', null);

  // Should be at request end.
  // Try to print something bogus. Should be undefined.
  VS(substr($o['text'], -7, 5), 'ended');
  $o = $c->processCmd('print', array('$foo'));
  VS(substr($o['text'], 23, 23), 'Undefined variable: foo');
  $o = $c->processCmd('continue', null);

  // Should be at psp end.
  // Try to print something bogus. Should be undefined.
  VS(substr($o['text'], -7, 5), 'ended');
  $o = $c->processCmd('print', array('$foo'));
  VS(substr($o['text'], 23, 23), 'Undefined variable: foo');

  // Tell the test harness that we're done messing with the request, and to
  // interrupt us one last time.
  $c->processCmd('break', array('clear', 'all'));
  sendToHarness('3');

  // Wait for an interrupt after the request is done, controlled by test harness
  $o = $c->processCmd('continue', null);
  VS($o['output_type'], 'code_loc');
  VS($o['file'], '');
}

try {
  signal1($client);
  signal2($client);
  echo PASS;
} catch (TestFailure $t) {
  error_log($t);
  echo FAIL;
} catch (Exception $e) {
  error_log($e);
  echo FAIL;
}
