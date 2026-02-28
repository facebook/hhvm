<?hh

class A {
    static public $a, $aa;
    static private $b, $bb;
    static protected $c, $cc;

    static public function test() :mixed{
        var_dump(get_class_vars(__CLASS__));
    }
}
<<__EntryPoint>> function main(): void {
var_dump(get_class_vars('A'));
var_dump(A::test());
}
