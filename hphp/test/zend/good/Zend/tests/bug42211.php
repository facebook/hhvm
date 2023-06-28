<?hh
class A {
    function foo() :mixed{
        var_dump(property_exists('B', 'publicBar'));
        var_dump(property_exists('B', 'protectedBar'));
        var_dump(property_exists('B', 'privateBar'));
    }
}

class B extends A {
    static public $publicBar = "ok";
    static protected $protectedBar = "ok";
    static private $privateBar = "fail";
}
<<__EntryPoint>> function main(): void {
$a = new A();
$a->foo();
$b = new B();
$b->foo();
}
