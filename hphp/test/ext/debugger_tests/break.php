<?php
require_once("hphpd.php");

error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("break", "debugger_tests");
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

function break1($c) {
  // file:line break
  $o = $c->processCmd('break', array('break_t.php:7'));
  VS($o['values'][0]['id'], 1);
  VS($o['values'][0]['state'], 'ALWAYS');
  VS($o['values'][0]['file'], 'break_t.php');
  VS($o['values'][0]['line1'], 7);
  $o = $c->processCmd('@', array('foo(\'test_break1\')'));
  VS($o['output_type'], 'code_loc');
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 7);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break1');
  VS($o['values']['y'], 'test_break1_suffix');
  testFin($c);
}

function break2($c) {
  // func() break
  $o = $c->processCmd('break', array('foo()'));
  VS($o['values'][0]['func'], 'foo');
  VS($o['values'][0]['class'], '');
  $o = $c->processCmd('@', array('foo(\'test_break2\')'));
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 6);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break2');
  VS($o['values']['y'], null);
  testFin($c);
}

function break3($c) {
  // object method() break
  $o = $c->processCmd('break', array('cls::pubObj()'));
  VS($o['values'][0]['func'], 'pubObj');
  VS($o['values'][0]['class'], 'cls');
  $c->processCmd('@', array('$break3=new cls()'));
  $o = $c->processCmd('@', array('$break3->pubObj(\'test_break3\')'));
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 12);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break3');
  testFin($c);
}

function break4($c) {
  // static method() break
  $o = $c->processCmd('break', array('cls::pubCls()'));
  VS($o['values'][0]['func'], 'pubCls');
  VS($o['values'][0]['class'], 'cls');
  $o = $c->processCmd('@', array('cls::pubCls(\'test_break4\')'));
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 15);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break4');
  testFin($c);
}

function break5($c) {
  // hphpd_break()
  $c->processCmd('@', array('$break5=new cls()'));
  $o = $c->processCmd('@', array('$break5->pubHardBreak(\'test_break5\')'));
  VS(substr($o['file'],-11), 'break_t.php');
  VS($o['line_no'], 19);
  testFin($c);
}

function break6_pre($c) {
  $c->processCmd('break', array('break_t2.php:7'));
  $c->processCmd('break', array('foo2()'));
  $c->processCmd('break', array('cls2::pubObj()'));
  $o = $c->processCmd('break', array('cls2::pubCls()'));
  VS($o['values'][0]['line1'], 7);
  VS($o['values'][1]['func'], 'foo2');
  VS($o['values'][2]['func'], 'pubObj');
  VS($o['values'][3]['func'], 'pubCls');
}

function break6_post($c) {
  $o = $c->processCmd('@', array('foo2(\'test_break6\')'));
  VS(substr($o['file'],-12), 'break_t2.php');
  VS($o['line_no'], 6);
  $o = $c->processCmd('continue', null);
  VS(substr($o['file'],-12), 'break_t2.php');
  VS($o['line_no'], 7);
  $c->processCmd('continue', null);
  $c->processCmd('@', array('$break6=new cls2()'));
  $o = $c->processCmd('@', array('$break6->pubObj(\'test_break6\')'));
  VS(substr($o['file'],-12), 'break_t2.php');
  VS($o['line_no'], 12);
  $c->processCmd('continue', null);
  $o = $c->processCmd('@', array('cls2::pubCls(\'test_break6\')'));
  VS(substr($o['file'],-12), 'break_t2.php');
  VS($o['line_no'], 15);
  testFin($c);
}

function break7($c) {
  // func() break
  $o = $c->processCmd('break', array('\\TestNs\\foo()'));
  VS($o['values'][0]['func'], 'TestNs\\foo');
  VS($o['values'][0]['class'], '');
  VS($o['values'][0]['namespace'], '');
  $o = $c->processCmd('@', array('\\TestNs\\foo(\'test_break7\')'));
  VS(substr($o['file'],-12), 'break_t3.php');
  VS($o['line_no'], 7);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break7');
  VS($o['values']['y'], null);
  testFin($c);
}

function break8($c) {
  // object method() break
  $o = $c->processCmd('break', array('TestNs\\cls::pubObj()'));
  VS($o['values'][0]['func'], 'pubObj');
  VS($o['values'][0]['class'], 'TestNs\\cls');
  VS($o['values'][0]['namespace'], '');
  $c->processCmd('@', array('$break8=new \\TestNs\\cls()'));
  $o = $c->processCmd('@', array('$break8->pubObj(\'test_break8\')'));
  VS(substr($o['file'],-12), 'break_t3.php');
  VS($o['line_no'], 13);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break8');
  testFin($c);
}

function break9($c) {
  // static method() break
  $o = $c->processCmd('break', array('\\TestNs\\cls::pubCls()'));
  VS($o['values'][0]['func'], 'pubCls');
  VS($o['values'][0]['class'], 'TestNs\\cls');
  VS($o['values'][0]['namespace'], '');
  $o = $c->processCmd('@', array('\\TestNs\\cls::pubCls(\'test_break9\')'));
  VS(substr($o['file'],-12), 'break_t3.php');
  VS($o['line_no'], 16);
  $o = $c->processCmd('variable', null);
  VS($o['values']['x'], 'test_break9');
  testFin($c);
}

function break10($c) {
  $o = $c->processCmd('break', array(':fb:my:thing::doIt()'));
  VS($o['text'], "Breakpoint 1 set upon entering xhp_fb__my__thing::doIt()\n");
  VS($o['values'][0]['func'], 'doIt');
  VS($o['values'][0]['class'], 'xhp_fb__my__thing');
  VS($o['values'][0]['namespace'], '');
  $o = $c->processCmd('@', array(':fb:my:thing::doIt()'));
  VS($o['line_no'], 8);
  $o = $c->processCmd('break', array('clear', 'all'));
  testFin($c);
}

try {
  // Test breakpoints on functions that are already seen
  $client->processCmd('@', array('include(\'break_t.php\')'));
  break1($client);
  break2($client);
  break3($client);
  break4($client);
  break5($client);
  // Test breakpoints on functions that are not seen yet
  break6_pre($client);
  $client->processCmd('@', array('include(\'break_t2.php\')'));
  break6_post($client);

  //Test breakpoints on function entry for classes in namespaces
  $client->processCmd('@', array('include(\'break_t3.php\')'));
  break7($client);
  break8($client);
  break9($client);

  //Test breakpoints on function entry for xhp classes
  $client->processCmd('@', array('include(\'break_t4.php\')'));
  break10($client);

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

