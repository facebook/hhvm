<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
    public static function bar(): void { echo "I am bar in C"; }
}

class D extends C {
    use T;
}

trait T {
    require this as C;

    public function foo(): void {
        self::bar();
    }
}

<<__EntryPoint>>
function main(): void {
    (new D())->foo();
}
