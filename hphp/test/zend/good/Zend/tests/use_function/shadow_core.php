<?hh

use function foo\strlen;
<<__EntryPoint>> function main(): void {
require 'includes/foo_strlen.inc';
var_dump(strlen('foo bar baz'));
echo "Done\n";
}
