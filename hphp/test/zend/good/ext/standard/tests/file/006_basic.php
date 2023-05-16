<?hh
/*
  Prototype: int fileperms ( string $filename );
  Description: Returns the permissions on the file, or FALSE in case of an error

  Prototype: bool chmod ( string $filename, int $mode );
  Description: Attempts to change the mode of the file specified by
    filename to that given in mode */
<<__EntryPoint>> function main(): void {

echo "*** Testing fileperms(), chmod() with files and dirs ***\n";
fopen(sys_get_temp_dir().'/'.'perm.tmp', "w");
var_dump( chmod(sys_get_temp_dir().'/'.'perm.tmp', 0755) ) ;
printf("%o", fileperms(sys_get_temp_dir().'/'.'perm.tmp')) ;
echo "\n";
clearstatcache();

mkdir(sys_get_temp_dir().'/'.'perm');
var_dump( chmod(sys_get_temp_dir().'/'.'perm', 0777) ) ;
printf("%o", fileperms(sys_get_temp_dir().'/'.'perm')) ;
echo "\n";
clearstatcache();

echo "Done\n";

unlink(sys_get_temp_dir().'/'.'perm.tmp');
rmdir(sys_get_temp_dir().'/'.'perm');
}
