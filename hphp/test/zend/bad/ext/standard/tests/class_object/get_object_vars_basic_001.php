<?php
/* Prototype  : proto array get_object_vars(object obj)
 * Description: Returns an array of object properties 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

Class A {
	private $hiddenPriv = 'A::hiddenPriv';

	public static function test($b) {
		echo __METHOD__ . "\n"; 
		var_dump(get_object_vars($b));
	}
}

Class B extends A {
	private $hiddenPriv = 'B::hiddenPriv';	
	private $priv = 'B::priv';
	protected $prot = 'B::prot';
	public $pub = 'B::pub';

	public static function test($b) {
		echo __METHOD__ . "\n";		
		var_dump(get_object_vars($b));
	} 
}

Class C extends B {
	private $hiddenPriv = 'C::hiddenPriv';
	
	public static function test($b) {
		echo __METHOD__ . "\n";		
		var_dump(get_object_vars($b));
	} 
}

Class X {
	public static function test($b) {
		echo __METHOD__ . "\n";		
		var_dump(get_object_vars($b));
	} 
}


$b = new B;
echo "\n---( Global scope: )---\n";
var_dump(get_object_vars($b));
echo "\n---( Declaring class: )---\n";
B::test($b);
echo "\n---( Subclass: )---\n";
C::test($b);
echo "\n---( Superclass: )---\n";
A::test($b);
echo "\n---( Unrelated class: )---\n";
X::test($b);
?>