<?php
class A {
	static public $pubC = "pubC in A";
	static protected $protC = "protC in A";
	static private $privC = "privC in A";
	
	static public $pubA = "pubA in A";
	static protected $protA = "protA in A";
	static private $privA = "privA in A";		
}

class B extends A {
	static public $pubC = "pubC in B";
	static protected $protC = "protC in B";
	static private $privC = "privC in B";

	static public $pubB = "pubB in B";
	static protected $protB = "protB in B";
	static private $privB = "privB in B";	
}

class C extends B {
	static public $pubC = "pubC in C";
	static protected $protC = "protC in C";
	static private $privC = "privC in C";
}

class X {
	static public $pubC = "pubC in X";
	static protected $protC = "protC in X";
	static private $privC = "privC in X";	
}

$myC = new C;
$rc = new ReflectionClass("C");

function showInfo($name) {
	global $rc, $myC;
	echo "--- (Reflecting on $name) ---\n";
	try {
		$rp = $rc->getProperty($name);
	} catch (Exception $e) {
		echo $e->getMessage() . "\n";
		return;
	}
	try {
		var_dump($rp);
		var_dump($rp->getValue($myC));
	} catch (Exception $e) {		
		echo $e->getMessage() . "\n";
		return;
	}		
}


showInfo("pubA");
showInfo("protA");
showInfo("privA");

showInfo("pubB");
showInfo("protB");
showInfo("privB");

showInfo("pubC");
showInfo("protC");
showInfo("privC");
showInfo("doesntExist");

showInfo("A::pubC");
showInfo("A::protC");
showInfo("A::privC");

showInfo("B::pubC");
showInfo("B::protC");
showInfo("B::privC");

showInfo("c::pubC");
showInfo("c::PUBC");
showInfo("C::pubC");
showInfo("C::protC");
showInfo("C::privC");

showInfo("X::pubC");
showInfo("X::protC");
showInfo("X::privC");
showInfo("X::doesntExist");

showInfo("doesntexist::doesntExist");

?>
