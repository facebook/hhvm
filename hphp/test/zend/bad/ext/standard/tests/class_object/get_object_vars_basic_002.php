<?php
/* Prototype  : proto array get_object_vars(object obj)
 * Description: Returns an array of object properties 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

Class A {
	private $hiddenPriv = 'A::hiddenPriv';

	public function testA($b) {
		echo __METHOD__ . "\n"; 
		var_dump(get_object_vars($b));
	} 
}

Class B extends A {
	private $hiddenPriv = 'B::hiddenPriv';	
	private $priv = 'B::priv';
	protected $prot = 'B::prot';
	public $pub = 'B::pub';

	public function testB($b) {
		echo __METHOD__ . "\n";		
		var_dump(get_object_vars($b));
	} 
}


$b = new B;
echo "\n---( Declaring class: )---\n";
$b->testB($b);
echo "\n---( Superclass: )---\n";
$b->testA($b);

?>