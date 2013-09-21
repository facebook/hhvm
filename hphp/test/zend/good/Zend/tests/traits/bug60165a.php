<?php

trait A {
    public function bar() {}
}

class MyClass {
    use A {
        nonExistent as barA;
    }
}
