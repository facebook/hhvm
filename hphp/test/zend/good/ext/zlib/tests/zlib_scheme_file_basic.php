<?hh <<__EntryPoint>> function main(): void {
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
$contents = file($srcFile);
var_dump($contents);
echo "===DONE===\n";
}
