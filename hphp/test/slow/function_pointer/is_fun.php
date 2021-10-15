<?hh

<<__DynamicallyCallable>>
function func() {}

function rfunc<reify T>(): void {}
function rfunc2<reify T, Ti>(): void {}
function gfunc<T>(): void {}

class A {
  public function f() {}
  public static function f_static() {}
  public static function rclsmeth<reify T>() {}
}

function positive_tests() {
  var_dump('==================== positive tests (true) ====================');
  var_dump(HH\is_fun(rfunc<int>));
  var_dump(HH\is_fun(rfunc2<int, _>));
  var_dump(HH\is_fun(rfunc2<int, string>));
}

function negative_tests() {
  $func = 'func';
  var_dump('==================== negative tests (false) ====================');
  var_dump(HH\is_fun(HH\class_meth('A', 'f_static')));
  var_dump(HH\is_fun(A::f_static<>));
  var_dump(HH\is_fun(A::rclsmeth<int>));
}

<<__EntryPoint>>
function main(): void {
  positive_tests();
  negative_tests();
}
