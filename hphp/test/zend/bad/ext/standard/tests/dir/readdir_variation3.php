<?php
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass a directory handle pointing to a directory that has a sub-directory
 * to test behaviour of readdir()
 */

echo "*** Testing readdir() : usage variations ***\n";

// include the file.inc for Function: function create_files()
chdir(dirname(__FILE__));
include(dirname(__FILE__)."/../file/file.inc");

$path_top = dirname(__FILE__) . '/readdir_variation3';
$path_sub = $path_top . '/sub_folder';
mkdir($path_top);
mkdir($path_sub);

create_files($path_top, 2);
create_files($path_sub, 2);

$dir_handle = opendir($path_top);
while(FALSE !== ($file = readdir($dir_handle))) {
	
	// different OS order files differently so will
	// store file names into an array so can use sorted in expected output
	$contents[] = $file;
}

// more important to check that all contents are present than order they are returned in
sort($contents);
var_dump($contents);

delete_files($path_top, 2);
delete_files($path_sub, 2);

closedir($dir_handle);
?>
===DONE===
<?php
$path_top = dirname(__FILE__) . '/readdir_variation3';
$path_sub = $path_top . '/sub_folder';
rmdir($path_sub);
rmdir($path_top);
?>