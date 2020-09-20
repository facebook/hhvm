<?hh <<__EntryPoint>> function main(): void {
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
$contents = file_get_contents($srcFile);
echo $contents;
echo "===DONE===\n";
}
