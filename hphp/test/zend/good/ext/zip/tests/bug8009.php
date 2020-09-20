<?hh <<__EntryPoint>> function main(): void {
$thisdir = dirname(__FILE__);
$src = $thisdir . "/bug8009.zip";
$filename = __SystemLib\hphp_test_tmppath('tmp8009.zip');
copy($src, $filename);

$zip = new ZipArchive();

if (!$zip->open($filename)) {
       exit("cannot open $filename\n");
}
$zip->addFromString("2.txt", "=)");
$zip->close();
unlink($filename);
echo "status: " . $zip->status . "\n";
echo "\n";
}
