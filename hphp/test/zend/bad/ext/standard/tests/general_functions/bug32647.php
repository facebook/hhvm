<?php
ini_set('display_errors', 1);

ini_set('error_reporting', 4095);


function foo()
{
  echo "foo!\n";
}

class bar
{
	function barfoo ()
	{ echo "bar!\n"; }
}

unset($obj);
register_shutdown_function(array($obj,""));            // Invalid
register_shutdown_function(array($obj,"some string")); // Invalid
register_shutdown_function(array(0,""));               // Invalid
register_shutdown_function(array('bar','foo'));        // Invalid
register_shutdown_function(array(0,"some string"));    // Invalid
register_shutdown_function('bar');                     // Invalid
register_shutdown_function('foo');                     // Valid
register_shutdown_function(array('bar','barfoo'));     // Invalid

$obj = new bar;
register_shutdown_function(array($obj,'foobar'));      // Invalid
register_shutdown_function(array($obj,'barfoo'));      // Valid

?>