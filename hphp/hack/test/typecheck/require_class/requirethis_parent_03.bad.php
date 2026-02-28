<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
    public static function bar(): void {}
}

class D extends C {
    use T;
}

trait T {
    require this as C;

    public function foo(): void {
        parent::bar();
    }
}
