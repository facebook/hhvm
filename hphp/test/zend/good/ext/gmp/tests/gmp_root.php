<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gmp_root()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(gmp_root(1000, 3));
var_dump(gmp_root(100, 3));
var_dump(gmp_root(-100, 3));

var_dump(gmp_root(1000, 4));
var_dump(gmp_root(100, 4));
var_dump(gmp_root(-100, 4));

var_dump(gmp_root(0, 3));
var_dump(gmp_root(100, 0));
var_dump(gmp_root(100, -3));
}
