<?php
Phar::interceptFileFuncs();
is_file();
is_link();
var_dump(is_file(__FILE__));

$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
copy(dirname(__FILE__) . '/tar/files/links.tar', $fname2);
$a = new PharData($fname2);
$b = $a->convertToExecutable(Phar::TAR, Phar::NONE, '.3.phar.tar');
unset($a);
Phar::unlinkArchive($fname2);
$b['foo/stat.php'] = '<?php
echo "is_link\n";
var_dump(is_link("./stat.php"),is_file("./stat.php"), is_link("./oops"), is_file("./oops"));
var_dump(is_link("testit/link"), filetype("testit/link"), filetype("testit"), is_file("testit/link"));
echo "not found\n";
var_dump(is_link("notfound"));
echo "dir\n";
var_dump(is_dir("./bar"), is_file("foo/bar/blah"));
?>';
$b->addEmptyDir('foo/bar/blah');
$b->setStub('<?php
include "phar://" . __FILE__ . "/foo/stat.php";
__HALT_COMPILER();');
include $fname3;
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar'); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar'); ?>