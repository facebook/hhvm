<?php
/* Prototype:  string tempnam ( string $dir, string $prefix );
   Description: Create file with unique file name.
*/

/* Passing invalid/non-existing args for $dir, 
     hence the unique files will be created in temporary dir */
 
echo "*** Testing tempnam() with invalid/non-existing directory names ***\n";
/* An array of names, which will be passed as a dir name */ 
$names_arr = array(
	/* Invalid args */ 
	-1,
	TRUE,
	FALSE,
	NULL,
	"",
	" ",
	"\0",
	array(),

	/* Non-existing dirs */
	"/no/such/file/dir", 
	"php"
);

for( $i=0; $i<count($names_arr); $i++ ) {
	echo "-- Iteration $i --\n";
	$file_name = tempnam($names_arr[$i], "tempnam_variation3.tmp");

	if( file_exists($file_name) ){

		echo "File name is => ";
		print($file_name);
		echo "\n";

		echo "File permissions are => ";
		printf("%o", fileperms($file_name) );
		echo "\n";

		echo "File created in => ";
		$file_dir = dirname($file_name);
		if (realpath($file_dir) == realpath(sys_get_temp_dir()) || realpath($file_dir."\\") == realpath(sys_get_temp_dir())) {
			echo "temp dir\n";
		} else {
			echo "unknown location\n";
		}
	} else {
		echo "-- File is not created --\n";
	}

	unlink($file_name);
}

echo "\n*** Done ***\n";
?>