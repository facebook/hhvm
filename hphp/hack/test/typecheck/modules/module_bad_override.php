//// A.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

class A {
    <<__Internal>>
    public function foobar(): void {}
}

//// B.php
<?hh

class B extends A {
    public function foobar(): void {}
}
