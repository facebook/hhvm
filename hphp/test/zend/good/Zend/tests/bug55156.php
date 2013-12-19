<?php

/** test */
namespace foo {
	function test() { }
	 
	$x = new \ReflectionFunction('foo\test');
	var_dump($x->getDocComment());	
	
	/** test1 */
	class bar { }
	
	/** test2 */
	class foo extends namespace\bar { }
	
	$x = new \ReflectionClass('foo\bar');
	var_dump($x->getDocComment());
	
	$x = new \ReflectionClass('foo\foo');
	var_dump($x->getDocComment());
}
	
?>