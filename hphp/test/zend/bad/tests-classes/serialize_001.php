<?php

class Test implements Serializable
{
	public $data;

	function __construct($data)
	{
		echo __METHOD__ . "($data)\n";
		$this->data = $data;
	}

	function serialize()
	{
		echo __METHOD__ . "({$this->data})\n";
		return $this->data;
	}

	function unserialize($serialized)
	{
		echo __METHOD__ . "($serialized)\n";
		$this->data = $serialized;
		var_dump($this);
	}
}

$tests = array('String', NULL, 42, false);

foreach($tests as $data)
{
	try
	{
		echo "==========\n";
		var_dump($data);
		$ser = serialize(new Test($data));
		var_dump(unserialize($ser));
	}
	catch(Exception $e)
	{
		echo 'Exception: ' . $e->getMessage() . "\n";
	}
}

?>
===DONE===
<?php exit(0); ?>