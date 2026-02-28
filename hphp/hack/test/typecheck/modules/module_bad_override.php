//// module_A.php
<?hh
new module A {}

//// module_D.php
<?hh
new module D {}

//// A.php
<?hh

module A;

abstract class A {
 internal function foobar(): void {}
 abstract internal function abstractFoobar(): void;
 internal int $foobar = 42;
}

//// B.php
<?hh

class B extends A {
    // OK, as we are widening visibility
    public function foobar(): void {}
    // OK, as we are widening visibility
    public function abstractFoobar(): void {}
    // OK, though somewhat unexpected for props to have non-invariant visibility
    public int $foobar = 13;
}

class C extends A {
    // Not OK since since we are narrowing visibility and not in the same module
    private function foobar(): void {}
    // Not OK since since we are narrowing visibility and not in the same module
    protected function abstractFoobar(): void {}
}

//// D.php
<?hh

module D;

class D extends A {
    // Not OK since we are in a different module
    internal function foobar(): void {}
    // Not OK since we are in a different module
    internal function abstractFoobar(): void {}
    //Not OK since we are in a different module
    internal int $foobar = 13;
}

//// E.php
<?hh

module A;

class E extends A {
    // Not OK since we are narrowing visibility even though we are in the same module
    protected function foobar(): void {}
    // OK since we are in the same module
    internal function abstractFoobar(): void {}
}
