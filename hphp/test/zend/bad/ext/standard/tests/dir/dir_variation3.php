<?php
/* 
 * Prototype  : object dir(string $directory[, resource $context])
 * Description: Directory class with properties, handle and class and methods read, rewind and close
 * Source code: ext/standard/dir.c
 */

/*
 * Providing various permissions to the directory to be opened and checking
 * to see if dir() function opens the directory successfully.
 */

echo "*** Testing dir() : different directory permissions ***";

// create the temporary directory
$file_path = dirname(__FILE__);
$dir_path = $file_path."/dir_variation3";
@mkdir($dir_path);

/* different values for directory permissions */
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

// Open directory with different permission values, read and close, expected: none of them to succeed.
for($count = 0; $count < count($permission_values); $count++) {
  echo "\n-- Iteration ".($count + 1)." --\n";

  // try to remove the dir if exists  & create 
  $file_path = dirname(__FILE__);
  $dir_path = $file_path."/dir_variation3";
  @chmod ($dir_path, 0777); // change dir permission to allow all operation
  @rmdir ($dir_path);  // try n delete the dir

  // create the dir now
  @mkdir($dir_path);

  // change the dir permisson to test dir on it
  var_dump( chmod($dir_path, $permission_values[$count]) );

  // try to get dir handle
  $d = dir($dir_path);
  var_dump($d);   // dump the handle 

  // try read directory, expected : false
  echo "-- reading contents --\n";
  var_dump($d->read());

  // close directory
  $d->close();
}

echo "Done";
?>
<?php
// deleting temporary directory
$file_path = dirname(__FILE__);
$dir_path = $file_path."/dir_variation3";
rmdir($dir_path);
?>