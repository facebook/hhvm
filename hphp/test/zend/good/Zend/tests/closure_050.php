<?hh

class A {
    function foo() :mixed{
        $f = function() {
            return static::class;
        };
        return $f();
    }
}

class B extends A {}
<<__EntryPoint>> function main(): void {
$b = new B;
var_dump($b->foo());
}
