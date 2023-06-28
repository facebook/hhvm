<?hh

class base {
    private function show() :mixed{
        echo "base\n";
    }
    function test() :mixed{
        $this->show();
    }
}

class derived extends base {
    function show() :mixed{
        echo "derived\n";
    }
    function test() :mixed{
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
