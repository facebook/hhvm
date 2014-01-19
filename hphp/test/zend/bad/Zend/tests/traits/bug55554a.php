<?php

// All constructors should be registered as such

trait TConstructor {
    public function constructor() {
        echo "ctor executed\n";
    }
}

class NewConstructor {
	use TConstructor {
	    constructor as __construct;
	}
}

class LegacyConstructor {
    use TConstructor {
        constructor as LegacyConstructor;
    }
}

echo "New constructor: ";
$o = new NewConstructor;

echo "Legacy constructor: ";
$o = new LegacyConstructor;
