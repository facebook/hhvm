<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.txt';
file_put_contents($fname2, 'a');
class myIterator implements Iterator
{
    var $a;
    var $count = 1;

    function next() {
        return (++$this->count < 3000) ? 'f' . $this->count : false;
    }
    function current() {
        if (($this->count % 100) === 0) {
            echo $this->count, "\n";
        }
        return $GLOBALS['fname2'];
    }
    function key() {
        return 'f' . $this->count;
    }
    function valid() {
        return $this->count < 3000;
    }
    function rewind() {
        $this->count = 1;
        return $GLOBALS['fname2'];
    }
}
try {
	chdir(dirname(__FILE__));
	$phar = new Phar($fname);
	$ret = $phar->buildFromIterator(new myIterator);
	foreach ($ret as $a => $val) {
		$ret[$a] = str_replace(dirname($fname2) . DIRECTORY_SEPARATOR, '*', $val);
	}
	var_dump($ret);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.txt');
__halt_compiler();
?>