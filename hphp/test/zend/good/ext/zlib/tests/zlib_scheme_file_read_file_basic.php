<?hh <<__EntryPoint>> function main(): void {
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
readfile($srcFile);
echo "===DONE===\n";
}
