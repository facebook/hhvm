<?hh
class A {
    public static $x = "A";
    function testit() :mixed{
        $this->v1 = new self;
        $this->v2 = new self;
    }
}

class B extends A {
    public static $x = "B";
    function testit() :mixed{
        parent::testit();
        $this->v3 = new self;
        $this->v4 = new parent;
        $this->v4 = static::$x;
    }
}
<<__EntryPoint>> function main(): void {
$t = new B();
$t->testit();
var_dump($t);
}
