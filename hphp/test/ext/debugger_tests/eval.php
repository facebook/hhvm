<?php
require_once("hphpd.php");

error_log("In test ".$_SERVER['PHP_SELF']);
$client = get_client("eval", "debugger_tests");
if (!$client) {
  echo FAIL;
  return;
}

function eval1($c) {
  $o = $c->processCmd('print', array('function_exists(\'test1\')'));
  VS($o['values']['value'], false);
  $c->processCmd('@', array('function test1($x){error_log($x);return $x+1;}'));
  $o = $c->processCmd('print', array('function_exists(\'test1\')'));
  VS($o['values']['value'], true);
  $c->processCmd('@', array('$eval1=test1(4)'));
  $o = $c->processcmd('print', array('$eval1'));
  VS($o['values']['value'], 5);
}

function eval2($c) {
  $o = $c->processCmd('print', array('class_exists(\'test2\')'));
  VS($o['values']['value'], false);
  $c->processCmd('@', array('class test2 { '.
                            '  public $a; '.
                            '  private $b; '.
                            '  public function ab() { '.
                            '    return $this->a . ":" . $this->b;} '.
                            '  public function callCls() { '.
                            '    $obj = new cls(); '.
                            '    return $obj->meth($this);} '.
                            '  private function seven() { return 7;}'.
                            '} '));
  $o = $c->processCmd('print', array('class_exists(\'test2\')'));
  VS($o['values']['value'], true);
  $c->processCmd('set', array('bac', 'off'));
  $c->processCmd('@', array('$eval2 = new test2()'));
  $c->processCmd('@', array('$eval2->a = 3'));
  $c->processCmd('@', array('$eval2->b = 4'));
  $o = $c->processCmd('print', array('$eval2->ab()'));
  VS($o['values']['value'], '3:');
  $c->processCmd('set', array('bac', 'on'));
  $c->processCmd('@', array('$eval2->b = 4'));
  $o = $c->processCmd('print', array('$eval2->ab()'));
  VS($o['values']['value'], '3:4');

  $c->processCmd('break', array('eval_t.php:12'));
  $o = $c->processCmd('print', array('$eval2->callCls()'));
  VS($o['output_type'], 'code_loc');
  VS($o['line_no'], 12);
  $c->processCmd('break', array('clear', 'all'));
  $c->processCmd('set', array('bac', 'off'));
  $c->processCmd('@', array('$this->pub = 21'));
  // should work as it's in the context
  $c->processCmd('@', array('$this->pri = 22'));
  $c->processCmd('next', null);
  // shouldn't work as seven() is not accessible
  $c->processCmd('@', array('$this->pub = $x->seven()'));
  $c->processCmd('set', array('bac', 'on'));
  // now seven() is accessible
  $c->processCmd('@', array('$this->pri = $x->seven()'));
  $o = $c->processCmd('continue', null);
  VS($o['output_type'], 'values');
  VS($o['values']['value'], '11:12-21:22-21:7');
}

try {
  $client->processCmd('@', array('include(\'eval_t.php\')'));
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
