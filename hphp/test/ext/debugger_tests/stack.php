<?php
require_once("hphpd.php");

error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("stack", "debugger_tests");
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

function stack1($c) {
  $c->processCmd('break', array('bar()'));
  $o = $c->processCmd('@', array('test(1, 0)'));
  VS(substr($o['file'],-11), 'stack_t.php');
  VS($o['line_no'], 6);
  $c->processCmd('break', array('clear', 'all'));

  $c->processCmd('set', array('sa', 'on'));
  $o = $c->processCmd('where', null);
  VS($o['frame'], 0);
  VS(count($o['stacktrace']), 7);
  VS(substr($o['stacktrace'][0]['file'],-11), 'stack_t.php');
  VS($o['stacktrace'][0]['line'], 6);
  VS($o['stacktrace'][1]['function'], 'bar');
  VS($o['stacktrace'][1]['line'], 16);
  VS($o['stacktrace'][1]['args'][0], 1);
  VS($o['stacktrace'][1]['args'][1], 0);
  VS($o['stacktrace'][2]['function'], 'foo');
  VS($o['stacktrace'][2]['line'], 28);
  VS($o['stacktrace'][3]['class'], 'cls');
  VS($o['stacktrace'][3]['function'], 'pri');
  VS($o['stacktrace'][3]['line'], 22);
  VS($o['stacktrace'][4]['class'], 'cls');
  VS($o['stacktrace'][4]['function'], 'pub');
  VS($o['stacktrace'][4]['line'], 39);

  $c->processCmd('out', null);
  $o = $c->processCmd('where', null);
  VS(count($o['stacktrace']), 6);
  VS(substr($o['stacktrace'][0]['file'],-11), 'stack_t.php');
  VS($o['stacktrace'][0]['line'], 16);

  $c->processCmd('out', null);
  $o = $c->processCmd('where', null);
  VS(count($o['stacktrace']), 5);
  VS(substr($o['stacktrace'][0]['file'],-11), 'stack_t.php');
  VS($o['stacktrace'][0]['line'], 28);

  $c->processCmd('set', array('sa', 'off'));
  $c->processCmd('out', null);
  $o = $c->processCmd('where', null);
  VS(count($o['stacktrace']), 4);
  VS(substr($o['stacktrace'][0]['file'],-11), 'stack_t.php');
  VS($o['stacktrace'][0]['line'], 22);
  VS(isset($o['stacktrace'][1]['args']), false);
  $c->processCmd('set', array('sa', 'on'));

  testFin($c);
}

function stack2($c) {
  $c->processCmd('break', array('stack_t.php:9'));
  $o = $c->processCmd('@', array('test(2, 2)'));
  VS(substr($o['file'],-11), 'stack_t.php');
  VS($o['line_no'], 9);

  $o = $c->processCmd('where', null);
  VS(count($o['stacktrace']), 15);
  VS(substr($o['stacktrace'][0]['file'],-11), 'stack_t.php');
  VS($o['stacktrace'][0]['line'], 9);
  VS($o['stacktrace'][2]['function'], 'foo');
  VS($o['stacktrace'][2]['args'][0], 2);
  VS($o['stacktrace'][2]['args'][1], 0);
  VS($o['stacktrace'][6]['function'], 'foo');
  VS($o['stacktrace'][6]['args'][0], 2);
  VS($o['stacktrace'][6]['args'][1], 1);
  VS($o['stacktrace'][10]['function'], 'foo');
  VS($o['stacktrace'][10]['args'][0], 2);
  VS($o['stacktrace'][10]['args'][1], 2);

  $o = $c->processCmd('frame', array('6'));
  VS(count($o['stacktrace']), 15);
  VS($o['frame'], 6);
  $o = $c->processCmd('variable', null);
  VS($o['values']['y'], 1);
  $o = $c->processCmd('up', null);
  VS($o['frame'], 7);
  $o = $c->processCmd('down', null);
  VS($o['frame'], 6);
  $c->processCmd('@', array('$x=3'));
  // will hit hphpd_break since $x is 3
  $o = $c->processCmd('continue', null);
  VS(substr($o['file'],-11), 'stack_t.php');
  VS($o['line_no'], 30);
  $o = $c->processCmd('where', null);
  VS(count($o['stacktrace']), 9);
  VS($o['stacktrace'][0]['line'], 30);
  VS($o['stacktrace'][0]['function'], 'hphpd_break');
  // hphpd break defaults its frame to 1
  VS($o['frame'], 1);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 3);
  testFin($c);
}

try {
  $client->processCmd('@', array('include(\'stack_t.php\')'));
  stack1($client);
  stack2($client);
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
