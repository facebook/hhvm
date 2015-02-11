<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

interface MyCollection 
{
	const MAX_NUMBER_ITEMS = 1000;

	function put($item);
	function get();
}
