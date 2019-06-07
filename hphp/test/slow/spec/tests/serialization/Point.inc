<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point implements Serializable
{
	public $x;
	public $y;

	public function __construct($x = 0, $y = 0)
	{
		$this->x = $x;
		$this->y = $y;
	}

	public function __toString()
	{
		return '(' . $this->x . ',' . $this->y . ')';
	}

	public function serialize()
	{
		return serialize( /*...*/ );
    }

	public function unserialize($data)
	{
//		... = unserialize($data);
	}
}
