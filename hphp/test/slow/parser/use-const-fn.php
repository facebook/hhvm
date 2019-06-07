<?hh

use const foo\bar, foo\baz, foo\quux;
use function foo\bar, foo\bax, foo\quux;

<<__EntryPoint>>
function main_use_const_fn() {
echo "No parse errors!\n";
}
