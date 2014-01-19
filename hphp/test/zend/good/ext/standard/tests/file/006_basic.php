<?php
/*
  Prototype: int fileperms ( string $filename );
  Description: Returns the permissions on the file, or FALSE in case of an error

  Prototype: bool chmod ( string $filename, int $mode );
  Description: Attempts to change the mode of the file specified by
    filename to that given in mode
*/
$path = dirname(__FILE__);

echo "*** Testing fileperms(), chmod() with files and dirs ***\n"; 
fopen($path."/perm.tmp", "w");
var_dump( chmod($path."/perm.tmp", 0755 ) );
printf("%o", fileperms($path."/perm.tmp") );
echo "\n";
clearstatcache();

mkdir($path."/perm");
var_dump( chmod( $path."/perm", 0777 ) );
printf("%o", fileperms($path."/perm") );
echo "\n";
clearstatcache();

echo "Done\n";
?>
<?php
unlink(dirname(__FILE__)."/perm.tmp");
rmdir(dirname(__FILE__)."/perm");
?>