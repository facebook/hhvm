<?php

class Test {
    const TEST = 'TEST';

    private $prop;

    public function readConst() {
        $this->prop = self::TEST;
    }
}

function doTest() {
    $obj = new Test;
    $obj->readConst();
    unset($obj);
    var_dump(Test::TEST);
}

doTest();
eval('class Test2 extends Test {}');
doTest();

?>
