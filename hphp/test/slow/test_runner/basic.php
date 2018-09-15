<?hh

namespace HHVM\TestRunner\TypecheckerMode\Tests;

function foo(): void {
  echo "Hello";
}

<<__EntryPoint>>
function main_basic() {
foo();
}
