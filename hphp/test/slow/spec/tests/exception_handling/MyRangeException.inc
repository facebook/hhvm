<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class MyRangeException extends Exception 
{
	private $badValue;
	private $lowerValue;
	private $upperValue;

	public function __construct($message, $badValue, $lowerValue, $upperValue)
	{
		parent::__construct($message);

		$this->badValue = $badValue;
		$this->lowerValue = $lowerValue;
		$this->upperValue = $upperValue;
	}

	public function getBadValue()   { return $this->badValue; }
	public function getLowerValue() { return $this->lowerValue; }
	public function getUpperValue() { return $this->upperValue; }

	public function __toString() 
	{
		return parent::__toString()
			. ", badValue: " . $this->badValue
			. ", lowerValue: " . $this->lowerValue
			. ", upperValue: " . $this->upperValue;
	}
}
