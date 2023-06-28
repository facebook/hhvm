<?hh

class A {
  public function f() :mixed{}
  public static function rclsmeth<reify T>() :mixed{}
  public static function rclsmeth2<reify T, Ti>() :mixed{}
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\class_meth_get_class(A::rclsmeth<int>));
  var_dump(HH\class_meth_get_class(A::rclsmeth2<int, _>));
  var_dump(HH\class_meth_get_class(A::rclsmeth2<int, string>));
}
