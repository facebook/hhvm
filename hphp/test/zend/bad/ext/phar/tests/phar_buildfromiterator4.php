<?php
class myIterator implements Iterator
{
    var $a;
    function __construct(array $a)
    {
        $this->a = $a;
    }
    function next() {
        echo "next\n";
        return next($this->a);
    }
    function current() {
        echo "current\n";
        return current($this->a);
    }
    function key() {
        echo "key\n";
        return key($this->a);
    }
    function valid() {
        echo "valid\n";
        return current($this->a);
    }
    function rewind() {
        echo "rewind\n";
        return reset($this->a);
    }
}
try {
	chdir(dirname(__FILE__));
	$phar = new Phar(dirname(__FILE__) . '/buildfromiterator.phar');
	var_dump($phar->buildFromIterator(new myIterator(
		array(
			'a' => basename(__FILE__, 'php') . 'phpt',
			// demonstrate that none of these are added
			'.phar/stub.php' => basename(__FILE__, 'php') . 'phpt',
			'.phar/alias.txt' => basename(__FILE__, 'php') . 'phpt',
			'.phar/oops' => basename(__FILE__, 'php') . 'phpt',
		))));
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/buildfromiterator.phar');
__HALT_COMPILER();
?>