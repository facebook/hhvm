<?hh
class A {
  public static function b() {
    return \STDIN;
  }
}

<<__EntryPoint>>
function main_stdin() {
var_dump(A::b());
}
