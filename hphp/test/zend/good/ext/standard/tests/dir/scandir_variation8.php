<?php
/* Prototype  : array scandir(string $dir [, int $sorting_order [, resource $context]])
 * Description: List files & directories inside the specified path 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass a directory containing files with different types of names to test how scandir()
 * reads them
 */

echo "*** Testing scandir() : usage variations ***\n";

$dir_path = dirname(__FILE__) . "/scandir_variation8/";
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
	fclose($$handle);
	$iterator++;
};

echo "\n-- Call to scandir() --\n";
var_dump($content = scandir($dir_path));

// remove all files in directory so can remove directory in CLEAN section
foreach ($content as $file_name) {
	// suppress errors as won't be able to remove "." and ".." entries
	@unlink($dir_path . $file_name);
}
?>
===DONE===
<?php
$dir_path = dirname(__FILE__) . "/scandir_variation8";
rmdir($dir_path);
?>