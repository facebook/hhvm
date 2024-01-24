<?hh

module b; // package bar

class B {
  // Error - package baz is not loaded by the same deployment as package bar
  <<__CrossPackage("baz")>>
  static function bar() : void {}
}

<<__Entrypoint>>
function main(): void {
  B::bar();
}
