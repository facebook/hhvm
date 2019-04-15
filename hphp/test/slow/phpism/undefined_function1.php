<?hh

namespace Foo;
use function Bar\f;
require_once 'undefined_function1.inc';

<<__EntryPoint>>
function main(): void {
  \var_dump(f());
}
