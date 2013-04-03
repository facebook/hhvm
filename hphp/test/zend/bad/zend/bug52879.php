<?php
class MyClass {
	public $myRef;
	public function __set($property,$value) {
		$this->myRef = $value;
	}
}
$myGlobal=new MyClass($myGlobal);
$myGlobal->myRef=&$myGlobal;
$myGlobal->myNonExistentProperty="ok\n";
echo $myGlobal;