<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

include_once 'Aircraft.inc';

class PassengerJet extends Aircraft 
{
	public function getMaxSpeed()
	{
		// implement method
		return 550;
	}

	public function getMaxAltitude()
	{
		// implement method
		return 30000;
	}

	// ...
}
