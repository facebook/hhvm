<?hh

class d {
    private function test2() :mixed{
        print "Bar\n";
    }
}

abstract class a extends d {
    public function test() :mixed{
        $this->test2();
    }
}

abstract class b extends a {
}

class c extends b {
    public function __construct() {
        $this->test();
    }
}
<<__EntryPoint>> function main(): void {
new c;
}
