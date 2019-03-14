<?hh

namespace Foo;
use function Bar\f;
require_once 'undefined_function_default.inc';

<<__EntryPoint>>
function main(): void {
  \var_dump(f());
}
