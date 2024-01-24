<?hh

module b; // package bar

class B {
  // Error: __CrossPackage accepts exactly one argument
  <<__CrossPackage("baz", 42)>>
  static function bar() : void {}
}

<<__Entrypoint>>
function main(): void {
  B::bar();
}
