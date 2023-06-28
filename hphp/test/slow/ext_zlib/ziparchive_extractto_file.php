<?hh

<<__EntryPoint>>
function main_ziparchive_extractto_file() :mixed{
$str = 'temp';
$dir = tempnam(sys_get_temp_dir(), __FILE__);
unlink($dir);
mkdir($dir);
$archive = new ZipArchive();
$archive->open("$dir/a.zip",ZipArchive::CREATE);
$archive->addFromString("../dir1/A.txt", $str);
$archive->addFromString("/var/www/dir2/B", $str);
$archive->addFromString("a/b/../../../dir3/C.txt.other", $str);
$archive->addFromString("a/b/../dir4/D.not.a.file/D.a.file", $str);
$archive->addFromString("a/b/../c/../../d/dir5/E", $str);
$archive->addFromString("./dir6/F.exe", $str);
$archive->addFromString("./G.txt", $str);
$archive->addFromString("H.txt", $str);
$archive->addFromString("x/y/dir7/../I.txt", $str);
$archive->addFromString("z/dir8/./J.txt", $str);
$archive->addFromString("SIMPLE.txt", $str);
$archive->close();
$archive2 = new ZipArchive();
$archive2->open("$dir/a.zip");
$archive2->extractTo($dir);
$archive2->close();
var_dump(file_exists("$dir/dir1/A.txt")); // true
var_dump(file_exists("../dir1/A.txt")); // false
var_dump(file_exists("$dir/var/www/dir2/B")); // true
var_dump(file_exists("/var/www/dir2/B")); // false
var_dump(file_exists("$dir/dir3/C.txt.other")); // true
var_dump(file_exists("a/b/../../../dir3/C.txt.other")); // false
var_dump(file_exists("$dir/a/dir4/D.not.a.file/D.a.file")); // true
var_dump(file_exists("a/b/../dir4/D.not.a.file/D.a.file")); // false
var_dump(file_exists("$dir/d/dir5/E")); // true
var_dump(file_exists("a/b/../c/../../d/dir5/E")); // false
var_dump(file_exists("$dir/dir6/F.exe")); // true
var_dump(file_exists("./dir6/F.exe")); // false
var_dump(file_exists("$dir/G.txt")); // true
var_dump(file_exists("./G.txt")); // false
var_dump(file_exists("$dir/H.txt")); // true
var_dump(file_exists("H.txt")); // false
var_dump(file_exists("$dir/x/y/I.txt")); // true
var_dump(file_exists("x/y/dir7/../I.txt")); // false
var_dump(file_exists("$dir/z/dir8/J.txt")); // true
var_dump(file_exists("z/dir8/./J.txt")); // false
var_dump(file_exists("$dir/SIMPLE.txt")); // true
var_dump(file_exists("SIMPLE.txt")); // false

// Cleanup. Also verifies that everything is where it is supposed to be.
unlink("$dir/dir1/A.txt");
rmdir("$dir/dir1");
unlink("$dir/var/www/dir2/B");
rmdir("$dir/var/www/dir2");
rmdir("$dir/var/www");
rmdir("$dir/var");
unlink("$dir/dir3/C.txt.other");
rmdir("$dir/dir3");
unlink("$dir/a/dir4/D.not.a.file/D.a.file");
rmdir("$dir/a/dir4/D.not.a.file");
rmdir("$dir/a/dir4");
rmdir("$dir/a");
unlink("$dir/d/dir5/E");
rmdir("$dir/d/dir5");
rmdir("$dir/d");
unlink("$dir/dir6/F.exe");
rmdir("$dir/dir6");
unlink("$dir/x/y/I.txt");
rmdir("$dir/x/y");
rmdir("$dir/x");
unlink("$dir/z/dir8/J.txt");
rmdir("$dir/z/dir8");
rmdir("$dir/z");
unlink("$dir/G.txt");
unlink("$dir/H.txt");
unlink("$dir/a.zip");
unlink("$dir/SIMPLE.txt");
rmdir($dir);
}
