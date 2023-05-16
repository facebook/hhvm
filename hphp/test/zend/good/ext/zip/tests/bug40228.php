<?hh <<__EntryPoint>> function main(): void {
$dest = sys_get_temp_dir();
$arc_name = dirname(__FILE__) . "/bug40228.zip";
$zip = new ZipArchive;
$zip->open($arc_name, ZipArchive::CREATE);;
$zip->extractTo($dest);
if (is_dir($dest . '/test/empty')) {
    echo "Ok\n";
    rmdir($dest . '/test/empty');
    rmdir($dest . '/test');
} else {
    echo "Failed.\n";
}
echo "Done\n";
}
