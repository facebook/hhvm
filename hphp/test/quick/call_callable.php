<?hh

class Functor {
  public function __invoke($x) {
    return intval($x);
  }
}

class Base {
  private function blah() { echo 'Base::blah', "\n"; }
  public function callInScope($x) {
    $x();
  }
}

class C extends Base {
  private function blah() { echo __CLASS__, "\n"; }
  public function exposeBlah() {
    return varray[$this, 'blah'];
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
    return intval(reset(inout $args));
  }
}


class Foo extends Base {
  public function get() { return varray[$this, 'blah']; }
}

function invoker($x) {
  call_user_func($x);
  $x();
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
  $call_static_arr = varray['C', 'intval'];
  $call_static_string = 'C::intval';
  $call_instance = varray[$inst, 'inst_intval'];
  $call_static_on_instance = varray[$inst, 'intval'];
  $call_closure = function($x) {return C::intval($x);};
  $call_magic_instance = varray[$inst_magic, 'inst_intval'];
  $call_invalid = varray['C', 'noSuchMethod'];

  echo "* call_user_func ********************\n";
  var_dump(call_user_func($call_func_string, $test));
  var_dump(call_user_func($call_functor, $test));
  var_dump(call_user_func($call_closure, $test));
  var_dump(call_user_func($call_static_string, $test));
  var_dump(call_user_func($call_static_arr, $test));
  var_dump(call_user_func($call_instance, $test));
  var_dump(call_user_func($call_static_on_instance, $test));
  var_dump(call_user_func($call_magic_instance, $test));

  echo "* ()-invoke ********************\n";
  var_dump($call_func_string($test));
  var_dump($call_functor($test));
  var_dump($call_closure($test));
  var_dump($call_static_arr($test));
  var_dump($call_instance($test));
  var_dump($call_static_on_instance($test));
  var_dump($call_magic_instance($test));

  var_dump($call_invalid($test)); // fatals
  var_dump($call_static_string($test)); // fatals
}

<<__EntryPoint>> function main(): void {
  test_inheritance();
  test_invocation_syntaxes();
}
