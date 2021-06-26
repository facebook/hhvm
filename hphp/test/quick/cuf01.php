<?hh

class C {
  <<__DynamicallyCallable>>
  public function foo($x, $y) {
    var_dump(isset($this));
    return $x + $y;
  }
  <<__DynamicallyCallable>>
  public static function bar($x, $y) {
    return $x + $y;
  }
}

<<__EntryPoint>>
function main(): void {
  $obj = new C;
  var_dump(call_user_func(varray[$obj, 'foo'], 123, 456));
  var_dump(call_user_func(varray[$obj, 'bar'], 123, 456));
  var_dump(call_user_func_array(varray[$obj, 'foo'], varray[123, 456]));
  var_dump(call_user_func_array(varray[$obj, 'bar'], varray[123, 456]));
  var_dump(call_user_func(varray['C', 'bar'], 123, 456));
  var_dump(call_user_func_array(varray['C', 'bar'], varray[123, 456]));
}
