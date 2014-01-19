<?php

trait A {
    public function bar() {}
}

class MyClass {
    use A {
        A::nonExistent as barA;
    }
}
