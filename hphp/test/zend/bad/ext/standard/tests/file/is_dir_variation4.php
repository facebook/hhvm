<?php
/* Prototype: bool is_dir ( string $dirname );
   Description: Tells whether the dirname is a directory
     Returns TRUE if the dirname exists and is a directory, FALSE  otherwise.
*/

/* Passing dir names with different notations, using slashes, wild-card chars */

$file_path = dirname(__FILE__);

echo "*** Testing is_dir() with different notations of dir names ***";
$dir_name = "/is_dir_variation4";
mkdir($file_path.$dir_name);

$dirs_arr = array(
  "is_dir_variation4",
  "./is_dir_variation4",

  /* Testing a file trailing slash */
  "is_dir_variation4/",
  "./is_dir_variation4/",

  /* Testing file with double trailing slashes */
  "is_dir_variation4//",
  "./is_dir_variation4//",
  ".//is_dir_variation4//",
  "is_dir_vari*",

  /* Testing Binary safe */
  "./is_dir_variation4/".chr(0),
  "is_dir_variation4\0"
);

$count = 1;
/* loop through to test each element the above array */
foreach($dirs_arr as $dir) {
  echo "\n-- Iteration $count --\n";
  var_dump( is_dir($file_path."/".$dir ) );
  $count++;
}

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$dir_name = $file_path."/is_dir_variation4";
rmdir($dir_name);
?>