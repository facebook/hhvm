<?hh <<__EntryPoint>> function main(): void {
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "file://$inputFileName";
$compressedFile = "compress.zlib://$srcFile";

echo "file=$compressedFile\n\n";
$h = fopen($compressedFile, 'r');
fpassthru($h);
fclose($h);
echo "===DONE===\n";
}
