<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace NS1;

<<__DynamicallyCallable>>
function f2()
{
	echo "Inside function " . __FUNCTION__ . "\n";
}
