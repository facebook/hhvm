<?hh

<<file:__EnableUnstableFeatures('modules')>>

<<__Module('A')>>
class A {
    <<__Internal>>
    public function foobar(): void {}
}

class B extends A {
    public function foobar(): void {}
}
