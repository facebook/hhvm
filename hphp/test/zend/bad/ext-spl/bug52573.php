<?php // test

$result = null;
$f = new SplFileObject(__FILE__, 'r');
var_dump($f->fscanf('<?php // %s', $result));
var_dump($result);
var_dump($f->fscanf('<?php // %s'));
?>