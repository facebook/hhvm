<?hh

use function foo\strlen;
<<__EntryPoint>> function main(): void {
require 'includes/foo_strlen.php';
var_dump(strlen('foo bar baz'));
echo "Done\n";
}
