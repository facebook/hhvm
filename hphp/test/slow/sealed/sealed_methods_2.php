<?hh

class D {}

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

<<__EntryPoint>>
function main(): void {
  new C();
  echo "done\n";
}
