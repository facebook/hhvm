<?hh
class A {
    public static function who() {
        echo "A\n";
    }
    public static function who2() {
        echo "A\n";
    }
}

class B extends A {
    public static function who() {
        echo "B\n";
    }
}

class C extends B {
    public function call($cb) {
        echo join('|', $cb) . "\n";
        $cb();
    }
    public function test() {
        $this->call(varray['parent', 'who']);
        $this->call(varray['C', 'parent::who']);
        $this->call(varray['B', 'parent::who']);
        $this->call(varray['E', 'parent::who']);
        $this->call(varray['A', 'who']);
        $this->call(varray['C', 'who']);
        $this->call(varray['B', 'who2']);
    }
}

class D {
    public static function who() {
        echo "D\n";
    }
}

class E extends D {
    public static function who() {
        echo "E\n";
    }
}

class O {
    public function who() {
        echo "O\n";
    }
}

class P extends O {
    function __toString() {
        return '$this';
    }
    public function who() {
        echo "P\n";
    }
    public function call($cb) {
        echo join('|', $cb) . "\n";
        $cb();
    }
    public function test() {
        $this->call(varray['parent', 'who']);
        $this->call(varray['P', 'parent::who']);
        $this->call(varray[$this, 'O::who']);
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
