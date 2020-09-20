<?hh

function f() {
  return ++EvalOrder1527::$a;
}

abstract final class EvalOrder1527 {
  public static $a;
}
<<__EntryPoint>> function main(): void {
var_dump(varray[EvalOrder1527::$a, f(), EvalOrder1527::$a]);
}
