<?hh

namespace ABC\QRS\XYZ {
  function foo() { echo "foo\n"; }
}

namespace {
  function main() {
    ALIAS\foo();
  }

  main();
}
