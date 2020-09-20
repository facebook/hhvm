<?hh
<<__EntryPoint>> function main(): void {
$str = file_get_contents(dirname(__FILE__).'/src.gd2');
$res = imagecreatefromstring($str);

/* string */
ob_start();
imagegd2($res);
$str2 = ob_get_clean();
var_dump(imagecreatefromstring($str2));

/* file */
$file = __SystemLib\hphp_test_tmppath('bug41442.gd2');
imagegd2($res, $file);
$str2 = file_get_contents($file);
var_dump(imagecreatefromstring($str2));

@unlink($file);

echo "Done\n";
}
