<?hh

class A {
    function foo() {
        $f = function() {
            return self::class;
        };
        return $f();
    }
}

class B extends A {}
<<__EntryPoint>> function main(): void {
$b = new B;
var_dump($b->foo());
}
