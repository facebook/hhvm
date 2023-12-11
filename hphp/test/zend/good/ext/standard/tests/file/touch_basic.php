<?hh
/* Prototype  : proto bool touch(string filename [, int time [, int atime]])
 * Description: Set modification time of file 
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing touch() : basic functionality ***\n";

$filename = sys_get_temp_dir().'/'.'touch_basic.dat';

echo "\n--- testing touch creates a file ---\n";
@unlink($filename);
if (file_exists($filename)) {
   die("touch_basic failed");
}
var_dump( touch($filename) );
if (file_exists($filename) == false) {
   die("touch_basic failed");
}

echo "\n --- testing touch doesn't alter file contents ---\n";
$testln = "Here is a test line";
$h = fopen($filename, "wb");
fwrite($h, $testln);
fclose($h);
touch($filename);
$h = fopen($filename, "rb");
echo fgets($h);
fclose($h);

echo "\n\n --- testing touch alters the correct file metadata ---\n";
$init_meta = stat($filename);
clearstatcache();
sleep(1);
touch($filename);
$next_meta = stat($filename);
$type = vec["dev", "ino", "mode", "nlink", "uid", "gid",
              "rdev", "size", "atime", "mtime", "ctime",
              "blksize", "blocks"];

for ($i = 0; $i < count($type); $i++) {
   if ($init_meta[$i] != $next_meta[$i]) {
      echo "stat data differs at $type[$i]\n";
   }
}


// Initialise all required variables
$time = 10000;
$atime = 20470;

// Calling touch() with all possible arguments
echo "\n --- testing touch using all parameters ---\n";
var_dump( touch($filename, $time, $atime) );
clearstatcache();
$init_meta = stat($filename);
echo "ctime=".$init_meta['ctime']."\n";
echo "mtime=".$init_meta['mtime']."\n";
echo "atime=".$init_meta['atime']."\n";

unlink($filename);

echo "Done";
}
