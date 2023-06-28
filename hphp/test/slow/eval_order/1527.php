<?hh

function f() :mixed{
  EvalOrder1527::$a ??=0;
  EvalOrder1527::$a += 1;
  return EvalOrder1527::$a;
}

abstract final class EvalOrder1527 {
  public static $a;
}
<<__EntryPoint>> function main(): void {
var_dump(varray[EvalOrder1527::$a, f(), EvalOrder1527::$a]);
}
