<?hh

namespace ABC\QRS\XYZ {
  function foo() { echo "foo\n"; }
}

namespace {
  <<__EntryPoint>> function main(): void {
    ALIAS\foo();
  }
}
