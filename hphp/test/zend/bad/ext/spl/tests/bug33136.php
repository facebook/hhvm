<?php

class Collection extends ArrayObject
{
	private $data;
	
	function __construct()
	{
		$this->data = array();
		parent::__construct($this->data);
	}
	
	function offsetGet($index)
	{
		echo __METHOD__ . "($index)\n";
		return parent::offsetGet($index);
	}
	
	function offsetSet($index, $value)
	{
		echo __METHOD__ . "(" . (is_null($index) ? "NULL" : $index) . ",$value)\n";
		parent::offsetSet($index, $value);
	}
}

echo "\n\nInitiate Obj\n";
$arrayObj = new Collection();

echo "Assign values\n";

$arrayObj[] = "foo";
var_dump($arrayObj[0]);

$arrayObj[] = "bar";
var_dump($arrayObj[0]);
var_dump($arrayObj[1]);

$arrayObj["foo"] = "baz";
var_dump($arrayObj["foo"]);

print_r($arrayObj);

var_dump(count($arrayObj));

?>
===DONE===
<?php //exit(0); ?>