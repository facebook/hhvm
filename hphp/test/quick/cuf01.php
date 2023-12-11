<?hh

class C {
  <<__DynamicallyCallable>>
  public function foo($x, $y) :mixed{
    var_dump(isset($this));
    return $x + $y;
  }
  <<__DynamicallyCallable>>
  public static function bar($x, $y) :mixed{
    return $x + $y;
  }
}

<<__EntryPoint>>
function main(): void {
  $obj = new C;
  var_dump(call_user_func(vec[$obj, 'foo'], 123, 456));
  var_dump(call_user_func(vec[$obj, 'bar'], 123, 456));
  var_dump(call_user_func_array(vec[$obj, 'foo'], vec[123, 456]));
  var_dump(call_user_func_array(vec[$obj, 'bar'], vec[123, 456]));
  var_dump(call_user_func(vec['C', 'bar'], 123, 456));
  var_dump(call_user_func_array(vec['C', 'bar'], vec[123, 456]));
}
