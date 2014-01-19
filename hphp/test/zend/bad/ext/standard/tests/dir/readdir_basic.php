<?php
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle 
 * Source code: ext/standard/dir.C
 */

/*
 * Test basic functionality of readdir()
 */

echo "*** Testing readdir() : basic functionality ***\n";

// include the file.inc for Function: function create_files()
chdir(dirname(__FILE__));
include(dirname(__FILE__)."/../file/file.inc");

$path = dirname(__FILE__) . '/readdir_basic';
mkdir($path);
create_files($path, 3);

echo "\n-- Call readdir() with \$path argument --\n";
var_dump($dh = opendir($path));
$a = array();
while( FALSE !== ($file = readdir($dh)) ) {
	$a[] = $file;
}
sort($a);
foreach($a as $file) {
	var_dump($file);
}

echo "\n-- Call readdir() without \$path argument --\n";
var_dump($dh = opendir($path));
$a = array();
while( FALSE !== ( $file = readdir() ) ) {
	$a[] = $file;
}
sort($a);
foreach($a as $file) {
	var_dump($file);
}

delete_files($path, 3);
closedir($dh);
?>
===DONE===
<?php
$path = dirname(__FILE__) . '/readdir_basic';
rmdir($path);
?>