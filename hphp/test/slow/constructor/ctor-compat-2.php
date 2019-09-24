<?hh
class Foo {}
interface I {
  public function __construct(Foo $x);
}
class D implements I {
  public function __construct(array $x) {}
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
