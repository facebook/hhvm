<?php
namespace foo {
use \foo;
class bar {
	function __construct() {echo __METHOD__,"\n";}
}
new foo;
new bar;
namespace oops;
class foo {
	function __construct() {echo __METHOD__,"\n";}
}
use foo\bar as foo1;
new foo1;
new foo;
}
?>
===DONE===