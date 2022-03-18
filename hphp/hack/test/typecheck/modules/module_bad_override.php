//// decls.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}
module D {}

//// A.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

abstract class A {
    <<__Internal>>
    public function foobar(): void {}
    <<__Internal>>
    abstract public function abstractFoobar(): void;
    <<__Internal>>
    public int $foobar = 42;
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
<<file:__EnableUnstableFeatures('modules'), __Module('D')>>

class D extends A {
    // Not OK
    <<__Internal>> public function foobar(): void {}
    // Not OK
    <<__Internal>> public function abstractFoobar(): void {}
    // Not OK
    <<__Internal>> public int $foobar = 13;
}
