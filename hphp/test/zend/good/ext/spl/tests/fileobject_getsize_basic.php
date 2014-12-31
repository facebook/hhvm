<?php
$file = __DIR__ ."/data.txt";
file_put_contents($file, "foobar");

$s = new SplFileObject( $file );
echo $s->getSize();
?>
<?php error_reporting(0); ?>
<?php
$file = __DIR__ ."/data.txt";
unlink($file);
?>