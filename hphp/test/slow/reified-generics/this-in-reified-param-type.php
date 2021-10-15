<?hh

class X<reify T> {}

class C {
  public function foo(X<this> $x): void { echo "foo\n"; }
  public static function bar(X<this> $x): void { echo "bar\n"; }
}

<<__EntryPoint>>
function main(): void {
  (new C())->foo(new X<C>());
  C::bar(new X<C>());
}
