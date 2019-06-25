<?hh
/*
   Prototype: void clearstatcache ([bool clear_realpath_cache[, filename]]);
   Description: clears files status cache
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing clearstatcache() function: error conditions ***\n";
try { var_dump( clearstatcache(0, "/foo/bar", 1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //No.of args more than expected
echo "*** Done ***\n";
}
