<?hh

/* Prototype  : string hash  ( string $algo  , string $data  [, bool $raw_output  ] )
 * Description: Generate a hash value (message digest)
 * Source code: ext/hash/hash.c
 * Alias to functions: */
<<__EntryPoint>> function main(): void {
echo "*** Testing hash() : error conditions ***\n";

echo "\n-- Testing hash() function with less than expected no. of arguments --\n";
try { var_dump(hash()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(hash('adler32')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing hash() function with more than expected no. of arguments --\n";
$extra_arg= 10;
try { var_dump(hash('adler32', '', false, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing hash() function with invalid hash algorithm --\n";
var_dump(hash('foo', ''));
echo "===Done===";
}
