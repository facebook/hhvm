<?hh

trait A {
   protected static function foo() {
 echo "abc\n";
 }
   private static function bar() {
 echo "def\n";
 }
}


class B {
   use A {
      A::foo as public;
      A::bar as public baz;
   }
}
<<__EntryPoint>> function main(): void {
B::foo();
B::baz();
}
