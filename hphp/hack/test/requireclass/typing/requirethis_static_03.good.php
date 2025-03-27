<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
    public static function bar(): void { echo "I am bar in C"; }
}

class D extends C {
    use T;
    public static function bar(): void { echo "I am bar in D"; }
}

trait T {
    require this as C;

    public function foo(): void {
        static::bar();
    }
}

<<__EntryPoint>>
function main(): void {
    (new D())->foo();
}
