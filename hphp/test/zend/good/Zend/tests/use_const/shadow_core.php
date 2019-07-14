<?hh

require 'includes/foo_php_version.php';

use const foo\PHP_VERSION;
<<__EntryPoint>> function main(): void {
var_dump(PHP_VERSION);
echo "Done\n";
}
