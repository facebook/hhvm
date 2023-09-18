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
    // Not OK
    protected function abstractFoobar(): void {}
}

//// D.php
<?hh

module D;

class D extends A {
    // Not OK
    internal function foobar(): void {}
    // Not OK
    internal function abstractFoobar(): void {}
    // Not OK
    internal int $foobar = 13;
}
