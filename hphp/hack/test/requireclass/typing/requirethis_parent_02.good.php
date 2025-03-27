<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class B {
    public static function bar(): void {}
}

class C extends B {
    use T;
}

trait T {
    require this as C;

    public function foo(): void {
        parent::bar();
    }
}
