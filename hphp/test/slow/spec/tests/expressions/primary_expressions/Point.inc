<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point 
{
	private $x;		// Cartesian x-coordinate
	private $y;		// Cartesian y-coordinate

	public function getX()	 	{ return $this->x; }
	public function setX($x)	{ $this->x = $x;   }
	public function getY()		{ return $this->y; }
	public function setY($y)	{ $this->y = $y;   }

//	public function __construct($x, $y) // see what happens if no default values
	public function __construct($x = 0, $y = 0) 
	{
		$this->x = $x;
		$this->y = $y;
	}

	public function move($x, $y) 
	{
		$this->x = $x;
		$this->y = $y;
	}	

	public function translate($x, $y) 
	{
		$this->x += $x;
		$this->y += $y;
	}

	public function __toString() 
	{
//		throw new Exception;	// throw statement is not permitted

		return '(' . $this->x . ',' . $this->y . ')';
	}	
}
