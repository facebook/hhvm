<?hh

namespace ABC\QRS\XYZ {
  function foo() :mixed{ echo "foo\n"; }
}

namespace {
  <<__EntryPoint>> function main(): void {
    ALIAS\foo();
  }
}
