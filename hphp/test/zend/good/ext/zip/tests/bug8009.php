<?hh <<__EntryPoint>> function main(): void {
$thisdir = dirname(__FILE__);
$src = $thisdir . "/bug8009.zip";
$filename = sys_get_temp_dir().'/'.'tmp8009.zip';
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
