<?php

class Test {
    public static function gen() {
        var_dump(get_class());
        var_dump(get_called_class());
        yield 1;
        yield 2;
        yield 3;
    }
}

class ExtendedTest extends Test {
}

foreach (ExtendedTest::gen() as $i) {
    var_dump($i);
}

?>