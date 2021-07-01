<?hh <<__EntryPoint>> function main(): void {
$dest = __SystemLib\hphp_test_tmproot();
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
