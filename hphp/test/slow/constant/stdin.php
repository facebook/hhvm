<?hh
class A {
  public static function b() {
    return \HH\stdin();
  }
}

<<__EntryPoint>>
function main_stdin() {
  var_dump(A::b());
}
