<?php
$fname = dirname(__FILE__) . '/tempmanifest1.phar.php';
$pname = 'phar://' . $fname;

$a = new Phar($fname);
$a['index.php'] = '<?php
Phar::mount("testit", dirname(Phar::running(0)) . "/testit");
echo file_get_contents(Phar::running(1) . "/testit/extfile.php"), "\n";
echo file_get_contents(Phar::running(1) . "/testit/directory"), "\n";
echo file_get_contents(Phar::running(1) . "/testit/existing.txt"), "\n";
include "testit/extfile.php";
include "testit/extfile2.php";
try {
Phar::mount(".phar/stub.php", dirname(Phar::running(0)) . "/testit/extfile.php");
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
?>';
$a['testit/existing.txt'] = 'oops';
$a->setStub('<?php
set_include_path("phar://" . __FILE__);
include "index.php";
__HALT_COMPILER();');
unset($a);
mkdir(dirname(__FILE__) . '/testit');
mkdir(dirname(__FILE__) . '/testit/directory');
file_put_contents(dirname(__FILE__) . '/testit/extfile.php', '<?php
var_dump(__FILE__);
?>');
file_put_contents(dirname(__FILE__) . '/testit/extfile2.php', '<?php
var_dump(__FILE__);
?>');
include dirname(__FILE__) . '/testit/extfile.php';
include $fname;

$a = opendir($pname . '/testit');
$out = array();
while (false !== ($b = readdir($a))) {
	$out[] = $b;
}
sort($out);
foreach ($out as $b) {
	echo "$b\n";
}
$out = array();
foreach (new Phar($pname . '/testit') as $b) {
	$out[] = $b->getPathName();
}
sort($out);
foreach ($out as $b) {
	echo "$b\n";
}
try {
Phar::mount($pname . '/testit', 'another\\..\\mistake');
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
try {
Phar::mount($pname . '/notfound', dirname(__FILE__) . '/this/does/not/exist');
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
try {
Phar::mount($pname . '/testit', dirname(__FILE__));
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
try {
Phar::mount($pname . '/testit/extfile.php', dirname(__FILE__));
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tempmanifest1.phar.php');
@unlink(dirname(__FILE__) . '/testit/extfile.php');
@unlink(dirname(__FILE__) . '/testit/extfile2.php');
@rmdir(dirname(__FILE__) . '/testit/directory');
@rmdir(dirname(__FILE__) . '/testit');

?>