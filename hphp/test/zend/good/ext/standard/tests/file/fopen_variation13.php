<?php
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

require_once('fopen_include_path.inc');

echo "*** Testing fopen() : variation ***\n";
$newpath = create_include_path();
set_include_path($newpath);
runtest();
$newpath = generate_next_path();
set_include_path($newpath);
runtest();

teardown_include_path();
restore_include_path();


function runtest() {
    $tempDir = 'fopen_variation13.dir.tmp';
	$tmpfile = 'fopen_variation13.tmp';
	$absFile = getcwd().'/'.$tempDir.'/'.$tmpfile;
	
	mkdir($tempDir);
	$h = fopen($absFile, "w", true);
	fwrite($h, "This is the test file");
	fclose($h);
	
	
	$h = fopen($absFile, "r");
	if ($h === false) {
	   echo "Not created absolute location\n";
	}
	else {
	   echo "Created in correct location\n";
	   fclose($h);
	}
    unlink($absFile);
    rmdir($tempDir);
	
}
?>
===DONE===