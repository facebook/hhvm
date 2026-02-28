<?hh

<<__EntryPoint>>
function main_ziparchive_extractto_directory() :mixed{
$dir = tempnam(sys_get_temp_dir(), __FILE__);
unlink($dir);
mkdir($dir);
$archive = new ZipArchive();
$archive->open("$dir/a.zip",ZipArchive::CREATE);
$archive->addEmptyDir("../dir1/");
$archive->addEmptyDir("/var/www/dir2/");
$archive->addEmptyDir("a/b/../../../dir3");
$archive->addEmptyDir("a/b/../dir4/");
$archive->addEmptyDir("a/b/../c/../../d/dir5/");
$archive->addEmptyDir("./dir6");
$archive->addEmptyDir("x/y/dir7/..");
$archive->addEmptyDir("z/dir8/.");
$archive->addEmptyDir("simple");
$archive->close();
$archive2 = new ZipArchive();
$archive2->open("$dir/a.zip");
$archive2->extractTo($dir);
$archive2->close();
var_dump(file_exists("$dir/dir1/")); // true
var_dump(file_exists("../dir1/")); // false
var_dump(file_exists("$dir/var/www/dir2")); // true
var_dump(file_exists("/var/www/dir2/")); // false
var_dump(file_exists("$dir/dir3/")); // true
var_dump(file_exists("a/b/../../../dir3/")); // false
var_dump(file_exists("$dir/a/dir4/")); // true
var_dump(file_exists("a/b/../dir4/")); // false
var_dump(file_exists("$dir/d/dir5/")); // true
var_dump(file_exists("a/b/../c/../../d/dir5/")); // false
var_dump(file_exists("$dir/dir6")); // true
var_dump(file_exists("./dir6")); // false
var_dump(file_exists("$dir/x/y/")); // true
var_dump(file_exists("x/y/dir7/..")); // false
var_dump(file_exists("$dir/z/dir8")); // true
var_dump(file_exists("z/dir8/.")); // false
var_dump(file_exists("$dir/simple")); // true
var_dump(file_exists("simple")); // false

// Cleanup. Also verifies that everything is where it is supposed to be.
rmdir("$dir/dir1");
rmdir("$dir/var/www/dir2");
rmdir("$dir/var/www");
rmdir("$dir/var");
rmdir("$dir/dir3");
rmdir("$dir/a/dir4");
rmdir("$dir/a");
rmdir("$dir/d/dir5");
rmdir("$dir/d");
rmdir("$dir/dir6");
rmdir("$dir/x/y");
rmdir("$dir/x");
rmdir("$dir/z/dir8");
rmdir("$dir/z");
rmdir("$dir/simple");
unlink("$dir/a.zip");
rmdir($dir);
}
