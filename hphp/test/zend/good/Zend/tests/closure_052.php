<?hh

class A {
    static function foo() {
        $f = function() {
            return static::class;
        };
        return $f();
    }
}

class B extends A {}
<<__EntryPoint>> function main(): void {
var_dump(B::foo());
}
