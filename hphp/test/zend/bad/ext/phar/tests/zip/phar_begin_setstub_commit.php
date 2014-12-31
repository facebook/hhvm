<?php
$p = new Phar(dirname(__FILE__) . '/brandnewphar.phar.zip', 0, 'brandnewphar.phar');
var_dump($p->isFileFormat(Phar::ZIP));
//var_dump($p->getStub());
var_dump($p->isBuffering());
$p->startBuffering();
var_dump($p->isBuffering());
$p['a.php'] = '<?php var_dump("Hello");';
$p->setStub('<?php var_dump("First"); Phar::mapPhar("brandnewphar.phar"); __HALT_COMPILER(); ?>');
include 'phar://brandnewphar.phar/a.php';
var_dump($p->getStub());
$p['b.php'] = '<?php var_dump("World");';
$p->setStub('<?php var_dump("Second"); Phar::mapPhar("brandnewphar.phar"); __HALT_COMPILER();');
include 'phar://brandnewphar.phar/b.php';
var_dump($p->getStub());
$p->stopBuffering();
echo "===COMMIT===\n";
var_dump($p->isBuffering());
include 'phar://brandnewphar.phar/a.php';
include 'phar://brandnewphar.phar/b.php';
var_dump($p->getStub());

// add portion to test setting stub from resource
file_put_contents(dirname(__FILE__) . '/myfakestub.php', '<?php var_dump("First resource"); Phar::mapPhar("brandnewphar.phar"); __HALT_COMPILER(); ?>');
$a = fopen(dirname(__FILE__) . '/myfakestub.php', 'rb');
$p->setStub($a);
var_dump($p->getStub());
$c = strlen('<?php var_dump("First resource"); Phar::mapPhar("brandnewphar.phar"); __HALT_COMPILER(); ?>');
file_put_contents(dirname(__FILE__) . '/myfakestub.php', '<?php var_dump("First resource"); Phar::mapPhar("brandnewphar.phar"); __HALT_COMPILER(); ?>' . 'extra stuff');
fseek($a, 0);
$p->setStub($a, $c);
var_dump($p->getStub());
fclose($a);
?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/brandnewphar.phar.zip');
unlink(dirname(__FILE__) . '/myfakestub.php');
?>