<?hh <<__EntryPoint>> function main(): void {
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$intervals = vec[7, 22, 54, 17, 27, 15, 1000];
// tell should be 7, 29, 83, 100, 127, 142, 176 (176 is length of uncompressed file)

var_dump(gztell($h));
foreach ($intervals as $interval) {
   gzread($h, $interval);
   var_dump(gztell($h));
}

gzclose($h);
echo "===DONE===\n";
}
