<?hh

namespace Foo;
use function Bar\f;

<<__EntryPoint>>
function main(): void {
  require_once 'undefined_function1.inc';
  \var_dump(f());
}
