<?php

/*
 *  Prototype: array stat ( string $filename );
 *  Description: Gives information about a file
 */

/* test the stats of dir/file when their names are stored in objects */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


/* create temp file and directory */
mkdir("$file_path/stat_variation7/");  // temp dir

$file_handle = fopen("$file_path/stat_variation7.tmp", "w");  // temp file
fclose($file_handle);


echo "\n*** Testing stat(): with filename
    and directory name stored inside a object ***\n";

// creating object with members as numeric and non-numeric filename and directory name
class object_temp {
public $var_name;
public function object_temp($name) {
$this->var_name = $name;
  }
}

// directory as member
$obj1 = new object_temp("$file_path/stat_variation7/");
$obj2 = new object_temp("$file_path/stat_variation7a/");

// file as member
$obj3 = new object_temp("$file_path/stat_variation7.tmp");
$obj4 = new object_temp("$file_path/stat_variation7a.tmp");

echo "\n-- Testing stat() on filename stored inside an object --\n";
var_dump( stat($obj3->var_name) );

$file_handle = fopen("$file_path/stat_variation7a.tmp", "w");
fclose($file_handle);
var_dump( stat($obj4->var_name) );

echo "\n-- Testing stat() on directory name stored inside an object --\n";
var_dump( stat($obj1->var_name) );

mkdir("$file_path/stat_variation7a/");
var_dump( stat($obj2->var_name) );

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/stat_variation7.tmp");
unlink("$file_path/stat_variation7a.tmp");
rmdir("$file_path/stat_variation7");
rmdir("$file_path/stat_variation7a");
?>