<?hh <<__EntryPoint>> function main(): void {
$image = imagecreate(1,1);// 1px image

$tempdir = sys_get_temp_dir(). '/php-gdtest' . rand();
if (!file_exists($tempdir) && !is_dir($tempdir)) {
    mkdir ($tempdir, 0777, true);
}

$userinput = "1\0"; // from post or get data
$temp = $tempdir. "/test" . $userinput .".tmp";

echo "\nimagegif TEST\n";
imagegif($image, $temp);
var_dump(file_exists($tempdir. "/test1"));
var_dump(file_exists($tempdir. "/test1.tmp"));
foreach (glob($tempdir . "/test*") as $file ) { unlink($file); }
error_reporting(0);
foreach (glob($tempdir . "/test*") as $file ) { unlink($file); }
rmdir($tempdir);
}
