<?php
$p=new Phar('bug53872-phar.phar');
$p->buildFromDirectory(__DIR__ . "/bug53872/");
$p->setStub('<?php __HALT_COMPILER();?\>');
$p->compressFiles(Phar::GZ);

print(file_get_contents('phar://bug53872-phar.phar/first.txt'));
print(file_get_contents('phar://bug53872-phar.phar/second.txt'));
print(file_get_contents('phar://bug53872-phar.phar/third.txt'));
?>
<?php
unlink("bug53872-phar.phar");
?>