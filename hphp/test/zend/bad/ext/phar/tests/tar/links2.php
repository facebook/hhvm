<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
$pname = 'phar://' . $fname;

include dirname(__FILE__) . '/files/corrupt_tarmaker.php.inc';
$a = new corrupt_tarmaker($fname, 'none');
$a->init();
$a->addFile('hardlink', 'internal/file.txt', array(
                    'mode' => 0xA000 + 0644,
                    'uid' => 0,
                    'gid' => 0,
                    'size' => 0,
                    'mtime' => time(),
                ));
$a->close();

try {
	$p = new PharData($fname);
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
?>