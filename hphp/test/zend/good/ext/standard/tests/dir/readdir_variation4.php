<?php
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass a directory handle pointing to a directory that contains 
 * files with different file names to test how readdir() reads them
 */

echo "*** Testing readdir() : usage variations ***\n";

$dir_path = dirname(__FILE__) . "/readdir_variation4/";
mkdir($dir_path);

// heredoc string
$heredoc = <<<EOT
hd_file
EOT;

$inputs = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,
       
       // empty data
/*10*/ "",
       array(),

       // string data
/*12*/ "double_file",
       'single_file',
       $heredoc,
);

$iterator = 1;
foreach($inputs as $key => $input) {
	echo "\n-- Iteration $iterator --\n";
	$handle = "fp{$iterator}";
	var_dump( $$handle = fopen(@"$dir_path$input.tmp", 'w') );
	var_dump( fwrite($$handle, $key));
	fclose($$handle);
	$iterator++;
};

echo "\n-- Call to readdir() --\n";
$dir_handle = opendir($dir_path);
while(FALSE !== ($file = readdir($dir_handle))){
	
	// different OS order files differently so will
	// store file names into an array so can use sorted in expected output
	$contents[] = $file;
	
	// remove files while going through directory
	@unlink($dir_path . $file);
}

// more important to check that all contents are present than order they are returned in
sort($contents);
var_dump($contents);

closedir($dir_handle);
?>
===DONE===
<?php
$dir_path = dirname(__FILE__) . "/readdir_variation4/";
rmdir($dir_path);
?>