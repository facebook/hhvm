<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$stub = b'<?php echo "first stub\n"; __HALT_COMPILER(); ?>';
$file = $stub;

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);
var_dump($stub);
var_dump($phar->getStub());
var_dump($phar->getStub() == $stub);

$stub = '<?php echo "second stub\n"; __HALT_COMPILER(); ?>';
$sexp = $stub . "\r\n";
$stub = fopen('data://,'.$stub, 'r');
$phar->setStub($stub);
var_dump($phar->getStub());
var_dump($phar->getStub() == $stub);
var_dump($phar->getStub() == $sexp);
$phar->stopBuffering();
var_dump($phar->getStub());
var_dump($phar->getStub() == $stub);
var_dump($phar->getStub() == $sexp);

$phar = new Phar($fname);
var_dump($phar->getStub() == $stub);
var_dump($phar->getStub() == $sexp);

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
__HALT_COMPILER();
?>