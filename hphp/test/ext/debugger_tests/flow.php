<?php
require_once("hphpd.php");

error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("flow", "debugger_tests");
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

function flow1($c) {
  // break in method entry and next until going out
  $c->processCmd('break', array('cls::pub()'));
  $o = $c->processCmd('@', array('test(1)'));
  VS(substr($o['file'],-10), 'flow_t.php');
  VS($o['line_no'], 12);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 13);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 1);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 14);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 62);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 63);
  testFin($c);
}

function flow2($c) {
  // break in a line in middle of a method and step into and out of a function
  $c->processCmd('break', array('flow_t.php:18'));
  $o = $c->processCmd('@', array('test(2)'));
  VS(substr($o['file'],-10), 'flow_t.php');
  VS($o['line_no'], 18);
  $o = $c->processCmd('step', null);
  VS($o['line_no'], 19);
  $o = $c->processCmd('variable', null);
  VS($o['values']['z'], 10);
  $o = $c->processCmd('step', null);
  VS($o['line_no'], 6);
  $o = $c->processCmd('variable', null);
  VS($o['values']['a'], 5);
  VS(isset($o['values']['c']), false);
  $o = $c->processCmd('step', null);
  VS($o['line_no'], 7);
  $o = $c->processCmd('variable', null);
  VS($o['values']['c'], 5);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 19);
  $o = $c->processCmd('step', null);
  VS($o['line_no'], 20);
  testFin($c);
}

function flow3($c) {
  // break in a line in middle of a function and continue to
  // hphpd_break() in parent
  $c->processCmd('break', array('flow_t.php:6'));
  $o = $c->processCmd('@', array('test(3)'));
  VS(substr($o['file'],-10), 'flow_t.php');
  VS($o['line_no'], 6);
  $o = $c->processCmd('continue', null);
  VS($o['line_no'], 21);
  testFin($c);
}

function flow4($c) {
  // break in a line in middle of a function and continue to
  // hphpd_break() in parent
  $c->processCmd('break', array('flow_t.php:7'));
  $o = $c->processCmd('@', array('test(4)'));
  VS(substr($o['file'],-10), 'flow_t.php');
  VS($o['line_no'], 7);
  $c->processCmd('break', array('clear', 'all'));
  $o = $c->processCmd('out', null);
  VS($o['line_no'], 19);
  $o = $c->processCmd('step', null);
  VS($o['line_no'], 20);
  $o = $c->processCmd('out', null);
  VS($o['line_no'], 13);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 14);
  $o = $c->processCmd('out', null);
  VS($o['line_no'], 62);
  testFin($c);
}

function flow5($c) {
  // break just before an exception is to be thrown, and step to the catch
  // block.
  $c->processCmd('break', array('flow_t.php:46'));
  $o = $c->processCmd('@', array('test(5)'));
  VS(substr($o['file'],-10), 'flow_t.php');
  VS($o['line_no'], 46);
  $c->processCmd('break', array('clear', 'all'));
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 50);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 53);
  $o = $c->processCmd('out', null);
  VS($o['line_no'], 27);
  testFin($c);
}

function flow6($c) {
  // break just before an exception is to be thrown, and out to the catch
  // block.
  $c->processCmd('break', array('flow_t.php:46'));
  $o = $c->processCmd('@', array('test(6)'));
  VS(substr($o['file'],-10), 'flow_t.php');
  VS($o['line_no'], 46);
  $c->processCmd('break', array('clear', 'all'));
  $o = $c->processCmd('out', null);
  VS($o['line_no'], 50);
  $o = $c->processCmd('next', null);
  VS($o['line_no'], 53);
  $o = $c->processCmd('out', null);
  VS($o['line_no'], 27);
  testFin($c);
}

function flow7($c) {
  // comfirm an up-stack breakpoint in a jitted basic block gets hit.
  $c->processCmd('break', array('flow_t.php:7'));
  $o = $c->processCmd('@', array('test(7)'));
  VS(substr($o['file'],-10), 'flow_t.php');
  VS($o['line_no'], 7);
  $c->processCmd('break', array('clear', 'all'));
  $c->processCmd('break', array('flow_t.php:62'));
  $o = $c->processCmd('continue', null);
  VS($o['line_no'], 62);
  testFin($c);
}

try {
  $client->processCmd('@', array('include(\'flow_t.php\')'));
  flow1($client);
  flow2($client);
  flow3($client);
  flow4($client);
  flow5($client);
  flow6($client);
  flow7($client);
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
