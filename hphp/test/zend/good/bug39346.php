<?php
class test
{
	protected $_id;
	static $instances;
	
	public function __construct($id) {
		$this->_id = $id;		
		self::$instances[$this->_id] = $this;
	}
	
	function __destruct() {
		unset(self::$instances[$this->_id]);
	}
}
$test = new test(2);
$test = new test(1);
$test = new test(2);
$test = new test(3);
echo "ok\n";
?>