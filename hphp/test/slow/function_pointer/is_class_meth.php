<?hh

<<__DynamicallyCallable>>
function func() :mixed{}

function rfunc<reify T>(): void {}

class A {
  public function f() :mixed{}
  public static function rclsmeth<reify T>() :mixed{}
  public static function rclsmeth2<reify T, Ti>() :mixed{}
}

function positive_tests() :mixed{
  var_dump('==================== positive tests (true) ====================');
  var_dump(HH\is_class_meth(A::rclsmeth<int>));
  var_dump(HH\is_class_meth(A::rclsmeth2<int, _>));
  var_dump(HH\is_class_meth(A::rclsmeth2<int, string>));
}

function negative_tests() :mixed{
  $func = 'func';
  var_dump('==================== negative tests (false) ====================');
  var_dump(HH\is_class_meth($func));
  var_dump(HH\is_class_meth(rfunc<int>));
  var_dump(HH\is_class_meth(meth_caller(A::class, 'f')));
}

<<__EntryPoint>>
function main(): void {
  positive_tests();
  negative_tests();
}
