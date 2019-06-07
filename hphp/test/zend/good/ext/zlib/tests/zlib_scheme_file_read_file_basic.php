<?hh <<__EntryPoint>> function main() {
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
readfile($srcFile);
echo "===DONE===\n";
}
