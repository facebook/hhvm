<?php

Phar::interceptFileFuncs();

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;

opendir(array());

mkdir(dirname(__FILE__) . '/poo');
chdir(dirname(__FILE__));

$a = opendir('poo');

$arr = array();
while (false !== ($b = readdir($a))) {
    $arr[] = $b;
}
sort($arr);
foreach ($arr as $b) {
    echo "$b\n";
}

closedir($a);

file_put_contents($pname . '/foo', '<?php
$context = stream_context_create();
$a = opendir(".", $context);
$res = array();
while (false !== ($b = readdir($a))) {
$res[] = $b;
}
sort($res);
foreach ($res as $b) {
echo "$b\n";
}
opendir("oops");
?>');

include $pname . '/foo';

?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>
<?php rmdir(dirname(__FILE__) . '/poo');