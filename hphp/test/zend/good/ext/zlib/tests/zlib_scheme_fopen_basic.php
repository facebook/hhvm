<?hh <<__EntryPoint>> function main(): void {
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
$h = fopen($srcFile, 'r');
fpassthru($h);
fclose($h);
echo "===DONE===\n";
}
