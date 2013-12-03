<?php
/* Prototype  : array scandir(string $dir [, int $sorting_order [, resource $context]])
 * Description: List files & directories inside the specified path 
 * Source code: ext/standard/dir.c
 */

/*
 * Create directories with different permissions to test whether scandir() can access them
 */

echo "*** Testing scandir() : usage variations ***\n";

// create the temporary directory
$dir_path = dirname(__FILE__) . "/scandir_variation7";
mkdir($dir_path);

// different values for directory permissions
$permission_values = array(
/*1*/  0477,  // owner has read only, other and group has rwx
       0677,  // owner has rw only, other and group has rwx

/*3*/  0444,  // all have read only
       0666,  // all have rw only

/*5*/  0400,  // owner has read only, group and others have no permission
       0600,   // owner has rw only, group and others have no permission

/*7*/  0470,  // owner has read only, group has rwx & others have no permission
       0407,  // owner has read only, other has rwx & group has no permission

/*9*/  0670,  // owner has rw only, group has rwx & others have no permission
/*10*/ 0607   // owner has rw only, group has no permission and others have rwx
);

$iterator = 1;
foreach ($permission_values as $perm) {
	echo "\n-- Iteration $iterator --\n";
	
	// Remove the directory if already exists
	if (is_dir($dir_path)){
		chmod ($dir_path, 0777); // change dir permission to allow all operation
		rmdir ($dir_path);
	}
	mkdir($dir_path);

	// change the dir permisson to test dir on it
	var_dump( chmod($dir_path, $perm) );
	
	var_dump(scandir($dir_path));
	$iterator++;
}
?>
===DONE===
<?php
$dir_path = dirname(__FILE__) . "/scandir_variation7";
rmdir($dir_path);
?>