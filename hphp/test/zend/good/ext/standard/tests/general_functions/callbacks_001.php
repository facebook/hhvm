<?hh
class A {
    public static function who() :mixed{
        echo "A\n";
    }
    public static function who2() :mixed{
        echo "A\n";
    }
}

class B extends A {
    public static function who() :mixed{
        echo "B\n";
    }
}

class C extends B {
    public function call($cb) :mixed{
        echo join('|', $cb) . "\n";
        $cb();
    }
    public function test() :mixed{
        $this->call(vec['parent', 'who']);
        $this->call(vec['C', 'parent::who']);
        $this->call(vec['B', 'parent::who']);
        $this->call(vec['E', 'parent::who']);
        $this->call(vec['A', 'who']);
        $this->call(vec['C', 'who']);
        $this->call(vec['B', 'who2']);
    }
}

class D {
    public static function who() :mixed{
        echo "D\n";
    }
}

class E extends D {
    public static function who() :mixed{
        echo "E\n";
    }
}

class O {
    public function who() :mixed{
        echo "O\n";
    }
}

class P extends O {
    function __toString() :mixed{
        return '$this';
    }
    public function who() :mixed{
        echo "P\n";
    }
    public function call($cb) :mixed{
        echo join('|', $cb) . "\n";
        $cb();
    }
    public function test() :mixed{
        $this->call(vec['parent', 'who']);
        $this->call(vec['P', 'parent::who']);
        $this->call(vec[$this, 'O::who']);
    }
}
<<__EntryPoint>> function main(): void {
$o = new C;
$o->test();

echo "===FOREIGN===\n";

$o = new P;
$o->test();

echo "===DONE===\n";
}
