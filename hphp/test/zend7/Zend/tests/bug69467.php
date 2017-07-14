<?php
interface Baz {
    public function bad();
}

trait Bar{
    protected function bad(){}
}

class Foo implements Baz{
    use Bar;
}

$test = new Foo();
var_dump($test instanceof Baz);
?>
