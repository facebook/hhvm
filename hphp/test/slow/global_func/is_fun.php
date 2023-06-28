<?hh

<<__DynamicallyCallable>>
function func() :mixed{}

function rfunc<reify T>(): void {}

function gfunc<T>(): void {}

class A {
  public function f() :mixed{}
  public static function f_static() :mixed{}
  public static function rclsmeth<reify T>() :mixed{}
}

function positive_tests() :mixed{
  $func = 'func';
  var_dump('==================== positive tests (true) ====================');
  var_dump(HH\is_fun(func<>));
  var_dump(HH\is_fun(HH\dynamic_fun($func)));
  var_dump(HH\is_fun(rfunc<int>));
  var_dump(HH\is_fun(gfunc<int>));
}

function negative_tests() :mixed{
  $func = 'func';
  var_dump('==================== negative tests (false) ====================');
  var_dump(HH\is_fun($func));
  var_dump(HH\is_fun(A::f_static<>));
  var_dump(HH\is_fun(A::rclsmeth<int>));
  var_dump(HH\is_fun($x ==> $x));
}
<<__EntryPoint>> function main(): void {
positive_tests();
negative_tests();
}
