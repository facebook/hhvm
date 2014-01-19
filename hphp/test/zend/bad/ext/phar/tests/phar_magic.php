<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$p = new Phar($fname);
$p['a'] = '<?php include "b/c.php";' . "\n";
$p['b/c.php'] = '<?php echo "in b\n";$a = fopen("a", "r", true);echo stream_get_contents($a);fclose($a);include dirname(__FILE__) . "/../d";';
$p['d'] = "in d\n";
$p->setStub('<?php
set_include_path("phar://" . __FILE__);
if (version_compare(PHP_VERSION, "5.3", "<")) {
Phar::interceptFileFuncs();
}
include "phar://" . __FILE__ . "/a";
__HALT_COMPILER();');
include $fname;
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
__HALT_COMPILER();
?>