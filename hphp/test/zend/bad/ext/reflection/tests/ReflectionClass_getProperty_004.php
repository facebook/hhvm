<?php
class A {
	public $pubC = "pubC in A";
	protected $protC = "protC in A";
	private $privC = "privC in A";
	
	public $pubA = "pubA in A";
	protected $protA = "protA in A";
	private $privA = "privA in A";		
}

class B extends A {
	public $pubC = "pubC in B";
	protected $protC = "protC in B";
	private $privC = "privC in B";

	public $pubB = "pubB in B";
	protected $protB = "protB in B";
	private $privB = "privB in B";	
}

class C extends B {
	public $pubC = "pubC in C";
	protected $protC = "protC in C";
	private $privC = "privC in C";
}

class X {
	public $pubC = "pubC in X";
	protected $protC = "protC in X";
	private $privC = "privC in X";	
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
