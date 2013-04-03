<?php

class C {
	public $a = "Original a";
	public $b = "Original b";
	public $c = "Original c";
	protected $d = "Original d";
	private $e = "Original e";

	function doForEachC() {
		echo "in C::doForEachC\n";
		foreach ($this as $k=>&$v) {
			var_dump($v);
			$v="changed.$k";
		}
	}
		
	static function doForEach($obj) {
		echo "in C::doForEach\n";
		foreach ($obj as $k=>&$v) {
			var_dump($v);
			$v="changed.$k";
		}
	}
	
	function doForEachOnThis() {
		echo "in C::doForEachOnThis\n";
		foreach ($this as $k=>&$v) {
			var_dump($v);
			$v="changed.$k";
		}
	}
	
}

class D extends C {
	
	private $f = "Original f";
	protected $g = "Original g";
	
	static function doForEach($obj) {
		echo "in D::doForEach\n";
		foreach ($obj as $k=>&$v) {
			var_dump($v);
			$v="changed.$k";
		}
	}

	function doForEachOnThis() {
		echo "in D::doForEachOnThis\n";
		foreach ($this as $k=>&$v) {
			var_dump($v);
			$v="changed.$k";
		}
	}
}

class E extends D {
	public $a = "Overridden a";
	public $b = "Overridden b";
	public $c = "Overridden c";
	protected $d = "Overridden d";
	private $e = "Overridden e";	

	static function doForEach($obj) {
		echo "in E::doForEach\n";
		foreach ($obj as $k=>&$v) {
			var_dump($v);
			$v="changed.$k";
		}
	}

	function doForEachOnThis() {
		echo "in E::doForEachOnThis\n";
		foreach ($this as $k=>&$v) {
			var_dump($v);
			$v="changed.$k";
		}
	}
}

echo "\n\nIterate over various generations from within overridden methods:\n";
echo "\n--> Using instance of C:\n";
$myC = new C;
$myC->doForEachOnThis();
var_dump($myC);
echo "\n--> Using instance of D:\n";
$myD = new D;
$myD->doForEachOnThis();
var_dump($myD);
echo "\n--> Using instance of E:\n";
$myE = new E;
$myE->doForEachOnThis();
var_dump($myE);

echo "\n\nIterate over various generations from within an inherited method:\n";
echo "\n--> Using instance of C:\n";
$myC = new C;
$myC->doForEachC();
var_dump($myC);
echo "\n--> Using instance of D:\n";
$myD = new D;
$myD->doForEachC();
var_dump($myD);
echo "\n--> Using instance of E:\n";
$myE = new E;
$myE->doForEachC();
var_dump($myE);

echo "\n\nIterate over various generations from within an overridden static method:\n";
echo "\n--> Using instance of C:\n";
$myC = new C;
C::doForEach($myC);
var_dump($myC);
$myC = new C;
D::doForEach($myC);
var_dump($myC);
$myC = new C;
E::doForEach($myC);
var_dump($myC);
echo "\n--> Using instance of D:\n";
$myD = new D;
C::doForEach($myD);
var_dump($myD);
$myD = new D;
D::doForEach($myD);
var_dump($myD);
$myD = new D;
E::doForEach($myD);
var_dump($myD);
echo "\n--> Using instance of E:\n";
$myE = new E;
C::doForEach($myE);
var_dump($myE);
$myE = new E;
D::doForEach($myE);
var_dump($myE);
$myE = new E;
E::doForEach($myE);
var_dump($myE);


echo "\n\nIterate over various generations from outside the object:\n";
echo "\n--> Using instance of C:\n";
$myC = new C;
foreach ($myC as $k=>&$v) {
	var_dump($v);
	$v="changed.$k";
}
var_dump($myC);
echo "\n--> Using instance of D:\n";
$myD = new D;
foreach ($myD as $k=>&$v) {
	var_dump($v);
	$v="changed.$k";
}
var_dump($myD);
echo "\n--> Using instance of E:\n";
$myE = new E;
foreach ($myE as $k=>&$v) {
	var_dump($v);
	$v="changed.$k";
}
var_dump($myE);
?>
===DONE===