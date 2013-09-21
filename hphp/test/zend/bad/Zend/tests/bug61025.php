<?php

Interface InvokeAble {
    static function __invoke();
}

class Bar {
    private function __invoke() {
        return __CLASS__;
    }
}

$b = new Bar;
echo $b();

echo $b->__invoke();

?>