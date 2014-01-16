<?php
Phar::interceptFileFuncs();
var_dump(stat(""));

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$a = new Phar($fname);
$a['my/index.php'] = '<?php
echo "stat\n";
var_dump(stat("dir/file1.txt"));
echo "lstat\n";
var_dump(lstat("dir/file1.txt"));
echo "fileperms\n";
var_dump(fileperms("dir/file1.txt"));
echo "fileinode\n";
var_dump(fileinode("dir/file1.txt"));
echo "filesize\n";
var_dump(filesize("dir/file1.txt"));
echo "fileowner\n";
var_dump(fileowner("dir/file1.txt"));
echo "filegroup\n";
var_dump(filegroup("dir/file1.txt"));
echo "filemtime\n";
var_dump(filemtime("dir/file1.txt"));
echo "fileatime\n";
var_dump(fileatime("dir/file1.txt"));
echo "filectime\n";
var_dump(filectime("dir/file1.txt"));
echo "filetype\n";
var_dump(filetype("dir/file1.txt"));
echo "is_writable\n";
var_dump(is_writable("dir/file1.txt"));
echo "is_writeable\n";
var_dump(is_writeable("dir/file1.txt"));
echo "is_readable\n";
var_dump(is_readable("dir/file1.txt"));
echo "is_executable\n";
var_dump(is_executable("dir/file1.txt"));
echo "file_exists\n";
var_dump(file_exists("dir/file1.txt"));
echo "is_dir\n";
var_dump(is_dir("dir/file1.txt"));
echo "is_file\n";
var_dump(is_file("dir/file1.txt"));
echo "is_link\n";
var_dump(is_link("dir/file1.txt"));
echo "not found\n";
var_dump(file_exists("not/found"));
echo "not found 2\n";
var_dump(fileperms("not/found"));
?>';
$a['dir/file1.txt'] = 'hi';
$a['dir/file2.txt'] = 'hi2';
$a['dir/file3.txt'] = 'hi3';
$a->setStub('<?php
set_include_path("phar://" . __FILE__ . "/dir" . PATH_SEPARATOR . "phar://" . __FILE__);
include "my/index.php";
__HALT_COMPILER();');
include $fname;
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>