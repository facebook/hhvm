<?hh

require 'includes/foo_strlen.php';

use function foo\strlen;
<<__EntryPoint>> function main(): void {
var_dump(strlen('foo bar baz'));
echo "Done\n";
}
