<?php
require_once("hphpd.php");
require_once("hphpd_test_inc.php");

error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("web_request", "debugger_tests");
if (!$client) {
  echo FAIL;
  return;
}

function breaks($c) {
  // order depends on web_request_t.php
  // file:line

  // We might like to test breaking at request start here, but we can't. There
  // is a race between telling the harness were ready for web_request_t.php
  // below and issuing the continue command right after that. The request may
  // start before we've issued the continue command, and we'll miss the
  // corresponding interrupt. We test request end and PSP end elsewhere, which
  // both share the same properties as request start, so that will have to do.

  // Normal breakpoint to get things started.
  $c->processCmd('break', array('break_t.php:7'));

  // Get the harness to load web_request_t.php, which will wait for the client
  // to enter the busy state (via the Continue command below) before proceeding
  // to execute the code where the breakpoint is set.
  sendToHarness('1');

  // Continue and wait for the breakpoint to hit.
  $o = $c->processCmd('continue', null);
  VS($o['output_type'], 'code_loc');
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 7);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break() in web_request_t.php');
  $c->processCmd('break', array('clear', 'all'));

  // func entry
  $c->processCmd('break', array('foo()'));
  $o = $c->processCmd('continue', null);
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 6);
  $c->processCmd('break', array('clear', 'all'));

  // object method
  $c->processCmd('break', array('cls::pubObj()'));
  $o = $c->processCmd('continue', null);
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 12);
  $c->processCmd('break', array('clear', 'all'));

  // static method
  $c->processCmd('break', array('cls::pubCls()'));
  $o = $c->processCmd('continue', null);
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 15);
  $c->processCmd('break', array('clear', 'all'));

  // hphpd_break()
  $o = $c->processCmd('continue', null);
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 19);
}

try {
  breaks($client);
  echo PASS;
} catch (TestFailure $t) {
  error_log($t);
  echo FAIL;
} catch (Exception $e) {
  error_log($e);
  echo FAIL;
}
