<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.php';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.3.phar.php';
$pname = 'phar://' . $fname;
$pname2 = 'phar://' . $fname2;
$pname3 = 'phar://' . $fname3;

// create in cwd
chdir(dirname(__FILE__));
file_put_contents('phar://fopen_edgetest.phar/hi', 'hi');
// append
$a = fopen($pname . '/b/c.php', 'a');
// invalid pharname
$a = fopen($pname . '.phar.gz', 'r');
// test phar_open_url() with quiet stat for code coverage
var_dump(file_exists($pname . '.phar.gz/hi'));
// test open for write with new phar
$a = fopen($pname . '/hi', 'w');
fclose($a);
// test open for write with corrupted phar
file_put_contents($fname2, '<?php oh crap __HALT_COMPILER();');
$a = fopen($pname2 . '/hi', 'w');
$a = fopen('phar://', 'r');
$a = fopen('phar://foo.phar', 'r');

file_put_contents($pname . '/hi', 'hi');
$a = fopen($pname . '/hi', 'r');
var_dump(fseek($a, 1), ftell($a));
var_dump(fseek($a, 1, SEEK_CUR), ftell($a));
fclose($a);

var_dump(stat('phar://'));
var_dump(stat('phar://foo.phar'));
var_dump(is_dir($pname));

// this tests coverage of the case where the phar exists and has no files
$phar = new Phar($fname3);
var_dump(file_exists($pname3 . '/test'));

unlink($pname2 . '/hi');
unlink('phar://');
unlink('phar://foo.phar');
unlink($pname . '/oops');

rename('phar://', 'phar://');
rename($pname . '/hi', 'phar://');
rename('phar://foo.phar/hi', 'phar://');
rename($pname . '/hi', 'phar://foo.phar/hi');

ini_set('phar.readonly', 1);
rename($pname . '/hi', $pname . '/there');
ini_set('phar.readonly', 0);
Phar::unlinkArchive($fname);
file_put_contents($pname . '/test.php', '<?php
$a = fopen("./notfound.php", "r");
?>');
include $pname . '/test.php';
?>

===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.php'); ?>
<?php unlink(dirname(__FILE__) . '/fopen_edgetest.phar');