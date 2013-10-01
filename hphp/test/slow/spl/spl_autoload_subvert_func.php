<?php
    function test(){
		class testCore {
		  public function __construct() {var_dump("myClass"); }
		}
		B::priv("zen");
    }	
    class A {
        public function __construct() {var_dump("construct A");
            spl_autoload_register(array($this, 'foo'));
        }
        private function foo($className) {echo 'Class A Trying to load ', $className, ' via ', __METHOD__, "()\n";
		if($className == "testCore"){test();}

        }
    }
    class B {
        public function __construct() {var_dump("construct B");
            spl_autoload_register(array($this, 'priv'));
        }
        private function priv($className) {
		echo 'Class B Trying to load ', $className, ' via ', __METHOD__, "()\n";
        }
    }
    $autoloader_A = new A();
    $autoloader_B = new B();
    $obj = new testCore();
?>
