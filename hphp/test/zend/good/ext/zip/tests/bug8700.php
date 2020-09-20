<?hh <<__EntryPoint>> function main(): void {
$thisdir = dirname(__FILE__);
$filename = $thisdir . "/bug8009.zip";

$zip = new ZipArchive();

if ($zip->open($filename) === FALSE) {
       exit("cannot open $filename\n");
}
$contents_from_idx = $zip->getFromIndex(0);
$contents_from_name = $zip->getFromName('1.txt');
if ($contents_from_idx != "=)" || $contents_from_name != "=)") {
    echo "failed:";
    var_dump($contents_from_idx, $contents_from_name);
}

$zip->close();
echo "status: " . $zip->status . "\n";
echo "\n";
}
