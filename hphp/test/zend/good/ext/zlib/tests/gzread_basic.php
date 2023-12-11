<?hh
// note that gzread is an alias to fread. parameter checking tests will be
// the same as fread
<<__EntryPoint>> function main(): void {
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$lengths = vec[10, 14, 7, 99, 2000];

foreach ($lengths as $length) {
   var_dump(gzread( $h, $length ) );
}
gzclose($h);

echo "===DONE===\n";
}
