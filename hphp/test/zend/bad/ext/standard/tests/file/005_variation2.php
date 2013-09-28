<?php
/*
   Prototype: int fileatime ( string $filename );
   Description: Returns the time the file was last accessed, or FALSE 
     in case of an error. The time is returned as a Unix timestamp.

   Prototype: int filemtime ( string $filename );
   Description: Returns the time the file was last modified, or FALSE 
     in case of an error. 

   Prototype: int filectime ( string $filename );
   Description: Returns the time the file was last changed, or FALSE
     in case of an error. The time is returned as a Unix timestamp.

   Prototype: bool touch ( string $filename [, int $time [, int $atime]] );
   Description: Attempts to set the access and modification times of the file
     named in the filename parameter to the value given in time.
*/

/*
   Prototype: void stat_fn(string $filename);
   Description: Prints access, modification and change times of a file
*/
function stat_fn( $filename ) {
  echo "\n-- File '$filename' --\n";
  echo "-- File access time is => "; 
  echo fileatime($filename)."\n";
  clearstatcache();
  echo "-- File modification time is => "; 
  echo filemtime($filename)."\n";
  clearstatcache();
  echo "-- inode change time is => "; 
  echo filectime($filename)."\n";
  clearstatcache();
  

}

echo "*** Testing fileattime(), filemtime(), filectime() & touch() : usage variations ***\n";
echo "\n*** testing touch ***\n";
$a = touch(NULL);
$b = touch(false);
$c = touch('');
$d = touch(' ');
$e = touch('|');

var_dump($a);
var_dump($b);
var_dump($c);
var_dump($d);
var_dump($e);

echo "\n*** testing file info ***";
stat_fn(NULL);
stat_fn(false);
stat_fn('');
stat_fn(' ');
stat_fn('|');

var_dump(unlink(' '));
var_dump(unlink('|'));

echo "Done";
?>