<?php

$fname = dirname(__FILE__) . '/tempmanifest1.phar.php';
$pname = 'phar://' . $fname;

$a = new Phar($fname);
$a['file1.txt'] = 'hi';
$a['file2.txt'] = 'hi2';
$a['subdir/ectory/file.txt'] = 'hi3';
$a->mount($pname . '/mount', __FILE__);
$a->addEmptyDir('one/level');

$a->extractTo(dirname(__FILE__) . '/extract', 'mount');
$a->extractTo(dirname(__FILE__) . '/extract');

$out = array();

foreach (new RecursiveIteratorIterator(new RecursiveDirectoryIterator(dirname(__FILE__) . '/extract', 0x00003000), RecursiveIteratorIterator::CHILD_FIRST) as $p => $b) {
	$out[] = $p;
}

sort($out);

foreach ($out as $b) {
	echo "$b\n";
}

$a->extractTo(dirname(__FILE__) . '/extract1', 'file1.txt');
var_dump(file_get_contents(dirname(__FILE__) . '/extract1/file1.txt'));

$a->extractTo(dirname(__FILE__) . '/extract1', 'subdir/ectory/file.txt');
var_dump(file_get_contents(dirname(__FILE__) . '/extract1/subdir/ectory/file.txt'));

$a->extractTo(dirname(__FILE__) . '/extract2', array('file2.txt', 'one/level'));
var_dump(file_get_contents(dirname(__FILE__) . '/extract2/file2.txt'));
var_dump(is_dir(dirname(__FILE__) . '/extract2/one/level'));

try {
	$a->extractTo(dirname(__FILE__) . '/whatever', 134);
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

$a->extractTo(array());

try {
	$a->extractTo('');
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

file_put_contents(dirname(__FILE__) . '/oops', 'I is file');

try {
	$a->extractTo(dirname(__FILE__) . '/oops', 'file1.txt');
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

try {
	$a->extractTo(dirname(__FILE__) . '/oops1', array(array(), 'file1.txt'));
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

try {
	$a->extractTo(dirname(__FILE__) . '/extract', 'file1.txt');
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

file_put_contents(dirname(__FILE__) . '/extract/file1.txt', 'first');
var_dump(file_get_contents(dirname(__FILE__) . '/extract/file1.txt'));

$a->extractTo(dirname(__FILE__) . '/extract', 'file1.txt', true);
var_dump(file_get_contents(dirname(__FILE__) . '/extract/file1.txt'));

try {
	$a->extractTo(str_repeat('a', 20000), 'file1.txt');
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

$a[str_repeat('a', 20000)] = 'long';

try {
	$a->extractTo(dirname(__FILE__) . '/extract', str_repeat('a', 20000));
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

?>
===DONE===
<?php
@rmdir(dirname(__FILE__) . '/whatever');
@unlink(dirname(__FILE__) . '/oops');
@rmdir(dirname(__FILE__) . '/oops1');
@unlink(dirname(__FILE__) . '/tempmanifest1.phar.php');
$e = dirname(__FILE__) . '/extract/';
@unlink($e . 'file1.txt');
@unlink($e . 'file2.txt');
@unlink($e . 'subdir/ectory/file.txt');
@rmdir($e . 'subdir/ectory');
@rmdir($e . 'subdir');
@rmdir($e . 'one/level');
@rmdir($e . 'one');
@rmdir($e);
$e = dirname(__FILE__) . '/extract1/';
@unlink($e . 'file1.txt');
@unlink($e . 'subdir/ectory/file.txt');
@rmdir($e . 'subdir/ectory');
@rmdir($e . 'subdir');
@rmdir($e);
$e = dirname(__FILE__) . '/extract2/';
@unlink($e . 'file2.txt');
@rmdir($e . 'one/level');
@rmdir($e . 'one');
@rmdir($e);
?>