<?php
trait TFoo {
    public function fooMethod(){}
}

class C {
    use TFoo {
        Typo::fooMethod as tf;
    }
}

echo "okey";
?>