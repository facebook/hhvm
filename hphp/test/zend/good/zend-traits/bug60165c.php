<?php

trait A {
    public function bar() {}
}

trait B {
    public function foo() {}
}

class MyClass {
    use A, B {
        foo as fooB;
        baz as foobar;
    }
}
