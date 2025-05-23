<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  protected internal function foo(): void {}
  public function bar(): void {}

  protected internal int $baz;
  public int $qux;
}

<<__EntryPoint>>
function main() {
   var_dump((new ReflectionMethod('A::foo')->isInternalToModule()));
   var_dump((new ReflectionMethod('A::bar')->isInternalToModule()));
   var_dump((new ReflectionProperty('A', 'baz')->isInternalToModule()));
   var_dump((new ReflectionProperty('A', 'qux')->isInternalToModule()));
}
