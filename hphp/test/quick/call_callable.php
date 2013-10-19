<?php

class Functor {
  public function __invoke($x) {
    return intval($x);
  }
}

class Base {
  private function blah() { echo 'Base::blah', "\n"; }
  public function callInScope($x) {
    call_user_func($x);
    call_user_func_array($x, array());
    $x();
  }
}

class C extends Base {
  private function blah() { echo __CLASS__, "\n"; }
  public function exposeBlah() {
    return array($this, 'blah');
  }

  public static function intval($x) {
    return intval($x);
  }
  public function inst_intval($x) {
    return self::intval($x);
  }
}

class CMagic {
  public function __call($name, $args) {
    echo $name, ' called on instance', "\n";
    return intval(reset($args));
  }
  public static function __callStatic($name, $args) {
    echo $name, ' called statically', "\n";
    return intval(reset($args));
  }
}

class Dtor {
  public function __destruct() { echo __METHOD__, "\n"; }
  public function f() { echo __METHOD__, "\n"; }
}


class Foo extends Base {
  public function get() { return array($this, 'blah'); }
}

function invoker($x) {
  call_user_func($x);
  $x();
}

function test_destructor() {
  echo ' = ', __FUNCTION__, " =\n";

  invoker(array(new Dtor(), 'f'));
  echo "done: test_destructor\n";
}

function test_inheritance() {
  echo ' = ', __FUNCTION__, " =\n";

  $x = new C();
  $x->callInScope($x->exposeBlah());
}

function test_invocation_syntaxes() {
  echo ' = ', __FUNCTION__, " =\n";

  $test = '10f';
  $call_functor = new Functor();
  $inst = new C();
  $inst_magic = new CMagic();
  $call_func_string = 'intval';
  $call_static_arr = array('C', 'intval');
  $call_static_string = 'C::intval';
  $call_instance = array($inst, 'inst_intval');
  $call_static_on_instance = array($inst, 'intval');
  $call_closure = function($x) {return C::intval($x);};
  $call_magic_static_arr = array('CMagic', 'intval');
  $call_magic_static_string = 'CMagic::intval';
  $call_magic_instance = array($inst_magic, 'inst_intval');
  $call_magic_closure = function($x) {return CMagic::intval($x);};
  $call_invalid = array('C', 'noSuchMethod');

  echo "* call_user_func ********************\n";
  var_dump(call_user_func($call_func_string, $test));
  var_dump(call_user_func($call_functor, $test));
  var_dump(call_user_func($call_closure, $test));
  var_dump(call_user_func($call_static_string, $test));
  var_dump(call_user_func($call_static_arr, $test));
  var_dump(call_user_func($call_instance, $test));
  var_dump(call_user_func($call_static_on_instance, $test));
  var_dump(call_user_func($call_magic_closure, $test));
  var_dump(call_user_func($call_magic_static_string, $test));
  var_dump(call_user_func($call_magic_static_arr, $test));
  var_dump(call_user_func($call_magic_instance, $test));

  echo "* ()-invoke ********************\n";
  var_dump($call_func_string($test));
  var_dump($call_functor($test));
  var_dump($call_closure($test));
  var_dump($call_static_arr($test));
  var_dump($call_instance($test));
  var_dump($call_static_on_instance($test));
  var_dump($call_magic_closure($test));
  var_dump($call_magic_static_arr($test));
  var_dump($call_magic_instance($test));

  var_dump($call_invalid($test)); // fatals
  var_dump($call_static_string($test)); // fatals
}

function main() {
  test_destructor();
  test_inheritance();
  test_invocation_syntaxes();
}
main();
