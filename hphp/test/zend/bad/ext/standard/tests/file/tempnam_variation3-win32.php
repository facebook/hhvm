<?php
/* Prototype:  string tempnam ( string $dir, string $prefix );
   Description: Create file with unique file name.
*/

/* Passing invalid/non-existing args for $prefix */

echo "*** Testing tempnam() with obscure prefixes ***\n";
$file_path = dirname(__FILE__)."/tempnamVar3";
if (!mkdir($file_path)) {
	echo "Failed, cannot create temp dir $filepath\n";
	exit(1);
}

$file_path = realpath($file_path);

/* An array of prefixes */ 
$names_arr = array(
	/* Valid args (casting)*/ 
	-1,
	TRUE,
	FALSE,
	NULL,
	"",
	" ",
	"\0",
	/* Invalid args */ 
	array(),

	/* Valid args*/ 
	/* prefix with path separator of a non existing directory*/
	"/no/such/file/dir", 
	"php/php"
);

$res_arr = array(
	/* Invalid args */ 
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,

	/* prefix with path separator of a non existing directory*/
	true, 
	true
);

for( $i=0; $i<count($names_arr); $i++ ) {
	echo "-- Iteration $i --\n";
	$file_name = tempnam($file_path, $names_arr[$i]);

	/* creating the files in existing dir */
	if (file_exists($file_name) && !$res_arr[$i]) {
		echo "Failed\n";
	}
	if ($res_arr[$i]) {
		$file_dir = dirname($file_name);
		if (realpath($file_dir) == $file_path || realpath($file_dir . "\\") == $file_path) {
			echo "OK\n";
		} else {
			echo "Failed, not created in the correct directory " . realpath($file_dir) . ' vs ' . $file_path ."\n";
		}
		
		if (!is_writable($file_name)) {
			printf("%o\n", fileperms($file_name) );

		}
	} else {
		echo "OK\n";
	}
	@unlink($file_name);
}

rmdir($file_path);
echo "\n*** Done. ***\n";
?>