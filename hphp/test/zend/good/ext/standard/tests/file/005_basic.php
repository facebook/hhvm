<?hh
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
<<__EntryPoint>> function main(): void {
echo "*** Testing the basic functionality with file ***\n";
print( @date('Y:M:D:H:i:s', fileatime(__FILE__)) )."\n";
print( @date('Y:M:D:H:i:s', filemtime(__FILE__)) )."\n";
print( @date('Y:M:D:H:i:s', filectime(__FILE__)) )."\n";
print( @date('Y:M:D:H:i:s', (int)touch(sys_get_temp_dir().'/'.'005_basic.tmp'))) ."\n";

echo "*** Testing the basic functionality with dir ***\n";
print( @date('Y:M:D:H:i:s', fileatime(".")) )."\n";
print( @date('Y:M:D:H:i:s', filemtime(".")) )."\n";
print( @date('Y:M:D:H:i:s', filectime(".")) )."\n";
print( @date('Y:M:D:H:i:s', (int)touch(sys_get_temp_dir().'/'.'005_basic'))) ."\n";

echo "\n*** Done ***\n";

unlink(sys_get_temp_dir().'/'.'005_basic.tmp');
unlink(sys_get_temp_dir().'/'.'005_basic');
}
