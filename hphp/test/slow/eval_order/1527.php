<?hh

function f() {
  return ++EvalOrder1527::$a;
}

abstract final class EvalOrder1527 {
  public static $a;
}
<<__EntryPoint>> function main() {
var_dump(array(EvalOrder1527::$a, f(), EvalOrder1527::$a));
}
