<?php

// The same is true for the insteadof operator to resolve conflicts

trait A {}

trait B {
    public function bar() {}
}

class MyClass {
    use A, B {
        A::bar insteadof B;
    }
}
