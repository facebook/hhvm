<?hh

class testClass {
    function __construct($x) {
        echo __METHOD__, " (". $x . ")\n";
    }
}

class testClass2 extends testClass {

  private static $__constructX = 0;
    function __construct() {

        if (self::$__constructX) {
            print "Infinite loop...\n";
        } else {
            self::$__constructX++;

            parent::__construct(1);
            testClass::__construct(2);
            parent::__construct(3);
            testClass::__construct(4);
        }
    }
}
<<__EntryPoint>> function main(): void {
new testClass2;
}
