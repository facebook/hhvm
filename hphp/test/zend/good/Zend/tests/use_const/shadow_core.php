<?hh

use const foo\PHP_VERSION;
<<__EntryPoint>> function main(): void {
require 'includes/foo_php_version.inc';
var_dump(PHP_VERSION);
echo "Done\n";
}
