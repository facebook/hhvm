<?php
ini_set('open_basedir', .);


$tmp_dir = __DIR__ . "/bug41518/";
@mkdir($tmp_dir);
$tmp_file = $tmp_dir."/bug41418.tmp";

touch($tmp_file);
var_dump(file_exists($tmp_file)); //exists
var_dump(file_exists($tmp_file."nosuchfile")); //doesn't exist

@unlink($tmp_file);
@rmdir($tmp_dir);
echo "Done\n";
?>
<?php
$tmp_dir = __DIR__ . "/bug41518/";
@unlink($tmp_dir);
?>