<?php

class test extends PDO
{
	protected function isProtected() {
		echo "this is a protected method.\n";
	}
	private function isPrivate() {
		echo "this is a private method.\n";
	}
    
    public function quote($str, $paramtype = NULL) {
    	$this->isProtected();
    	$this->isPrivate();
    	print $str ."\n";
	}
}

$test = new test('sqlite::memory:');
$test->quote('foo');
$test->isProtected();

?>