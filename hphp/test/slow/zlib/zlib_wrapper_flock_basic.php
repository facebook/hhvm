<?hh
// Based on ext/zlib/tests/zlib_wrapper_flock_basic.php
// except that locking actually works in HHVM, where it fails in PHP
<<__EntryPoint>> function main(): void {
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f,'r');
$wouldblock = false;
var_dump(flock($h, LOCK_SH, inout $wouldblock));
gzclose($h);

echo "===DONE===\n";
}
