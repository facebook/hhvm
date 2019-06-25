<?hh
// Based on ext/zlib/tests/zlib_wrapper_flock_basic.php
// except that locking actually works in HHVM, where it fails in PHP
<<__EntryPoint>> function main(): void {
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f,'r');
var_dump(flock($h, LOCK_SH));
gzclose($h);

echo "===DONE===\n";
}
