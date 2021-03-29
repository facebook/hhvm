<?hh
namespace A;
function foo(): void {
  \var_dump(__FUNCTION__);
}

namespace B;

use function C\foo;

namespace C;

use function A\foo;

<<__EntryPoint>>
function main(): void {
  foo();
}
