<?hh

class aParent {
    public static function staticTest() :mixed{
        $a = new A;
        var_dump(property_exists($a, "prot"));
        var_dump(property_exists($a, "prot2"));
        var_dump(property_exists($a, "prot3"));
        print "------------------\n";
        var_dump(property_exists("A", "prot"));
        var_dump(property_exists("A", "prot2"));
        var_dump(property_exists("A", "prot3"));
        print "------------------\n";
    }
    public function nonstaticTest() :mixed{
        $a = new A;
        var_dump(property_exists($a, "prot"));
        var_dump(property_exists($a, "prot2"));
        var_dump(property_exists($a, "prot3"));
        print "------------------\n";
        var_dump(property_exists("A", "prot"));
        var_dump(property_exists("A", "prot2"));
        var_dump(property_exists("A", "prot3"));
    }
}

class A extends aParent {
    static public $prot = "prot";
    static protected $prot2 = "prot";
    static private $prot3 = "prot";
}
<<__EntryPoint>> function main(): void {
A::staticTest();

$a = new A;
$a->nonstaticTest();
}
