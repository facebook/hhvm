<?php

trait TConstructor {
    public function foo() {
        echo "foo executed\n";
    }
    public function bar() {
        echo "bar executed\n";
    }
}

class OverridingIsSilent1 {
    use TConstructor {
	    foo as __construct;
	}
	
	public function __construct() {
	    echo "OverridingIsSilent1 __construct\n";
	}
}

$o = new OverridingIsSilent1;

class OverridingIsSilent2 {
    use TConstructor {
	    foo as OverridingIsSilent2;
	}
	
	public function OverridingIsSilent2() {
	    echo "OverridingIsSilent2 OverridingIsSilent2\n";
	}
}

$o = new OverridingIsSilent2;

class ReportCollision {
	use TConstructor {
	    bar as ReportCollision;
	    foo as __construct;
	}
}


echo "ReportCollision: ";
$o = new ReportCollision;

