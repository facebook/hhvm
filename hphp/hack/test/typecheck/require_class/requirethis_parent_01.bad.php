<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
    use T;

    public static function bar(): void {}
}

trait T {
    require this as C;

    public function foo(): void {
        parent::bar();
    }
}
