<?php
mkdir("bug45181_x");
var_dump(is_dir("bug45181_x"));
$origdir = getcwd();
chdir("bug45181_x");
var_dump(is_dir("bug45181_x"));
?>
<?php error_reporting(0); ?>
<?php
rmdir($origdir . "/bug45181_x");
?>
