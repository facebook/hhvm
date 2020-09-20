<?hh
/* Prototype: bool fnmatch ( string $pattern, string $string [, int $flags] )
   Description: fnmatch() checks if the passed string would match
     the given shell wildcard pattern.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing fnmatch() with file ***\n";
$file = basename(__FILE__);

var_dump( fnmatch("*.php", $file) );
var_dump( fnmatch("*.p*p", $file) );
var_dump( fnmatch("*.p*", $file) );
var_dump( fnmatch("*", $file) );
var_dump( fnmatch("**", $file) );
var_dump( fnmatch("*.phpt", $file) );

echo "*** Testing fnmatch() with other than file ***\n";
var_dump( fnmatch('100', '100') );
var_dump( fnmatch("string", "string") );
var_dump( fnmatch('1', '1') );
var_dump( fnmatch('', '') );

echo "\n*** Done ***\n";
}
