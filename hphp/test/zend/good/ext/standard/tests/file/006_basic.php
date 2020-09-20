<?hh
/*
  Prototype: int fileperms ( string $filename );
  Description: Returns the permissions on the file, or FALSE in case of an error

  Prototype: bool chmod ( string $filename, int $mode );
  Description: Attempts to change the mode of the file specified by
    filename to that given in mode */
<<__EntryPoint>> function main(): void {

echo "*** Testing fileperms(), chmod() with files and dirs ***\n";
fopen(__SystemLib\hphp_test_tmppath('perm.tmp'), "w");
var_dump( chmod(__SystemLib\hphp_test_tmppath('perm.tmp'), 0755 ) );
printf("%o", fileperms(__SystemLib\hphp_test_tmppath('perm.tmp')) );
echo "\n";
clearstatcache();

mkdir(__SystemLib\hphp_test_tmppath('perm'));
var_dump( chmod(__SystemLib\hphp_test_tmppath('perm'), 0777 ) );
printf("%o", fileperms(__SystemLib\hphp_test_tmppath('perm')) );
echo "\n";
clearstatcache();

echo "Done\n";

unlink(__SystemLib\hphp_test_tmppath('perm.tmp'));
rmdir(__SystemLib\hphp_test_tmppath('perm'));
}
