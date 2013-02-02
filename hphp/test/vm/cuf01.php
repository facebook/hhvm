<?php
class C {
  public function foo($x, $y) {
    var_dump(isset($this));
    return $x + $y;
  }
  public static function bar($x, $y) {
    return $x + $y;
  }
}

function main() {
  $obj = new C;
  var_dump(call_user_func(array($obj, 'foo'), 123, 456));
  var_dump(call_user_func(array($obj, 'bar'), 123, 456));
  var_dump(call_user_func_array(array($obj, 'foo'), array(123, 456)));
  var_dump(call_user_func_array(array($obj, 'bar'), array(123, 456)));
  var_dump(call_user_func(array('C', 'foo'), 123, 456));
  var_dump(call_user_func(array('C', 'bar'), 123, 456));
  var_dump(call_user_func_array(array('C', 'foo'), array(123, 456)));
  var_dump(call_user_func_array(array('C', 'bar'), array(123, 456)));
}
main();

