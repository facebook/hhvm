<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point implements Serializable 
{
	private static $pointCount = 0;

	public $x;
	public $y;

	public static function getPointCount() 
	{
		return self::$pointCount;
	}

	public function __construct($x = 0, $y = 0) 
	{
		$this->x = $x;
		$this->y = $y;
		++self::$pointCount;
	}

	public function __destruct() 
	{
		--self::$pointCount;
	}
///*
	public function __clone() 
	{
		++self::$pointCount;

//		echo "Inside " . __METHOD__ . ", point count = " . self::$pointCount . "\n";
	}
//*/

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
