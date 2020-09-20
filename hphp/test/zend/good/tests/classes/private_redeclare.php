<?hh

class base {
    private function show() {
        echo "base\n";
    }
    function test() {
        $this->show();
    }
}

class derived extends base {
    function show() {
        echo "derived\n";
    }
    function test() {
        echo "test\n";
        $this->show();
        parent::test();
        parent::show();
    }
}

<<__EntryPoint>> function main(): void {
$t = new base();
$t->test();

$t = new derived();
$t->test();
}
