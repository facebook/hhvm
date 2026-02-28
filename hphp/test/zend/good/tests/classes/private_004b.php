<?hh

class pass {
    private function show() :mixed{
        echo "Call show()\n";
    }

    public function do_show() :mixed{
        $this->show();
    }
}

class fail extends pass {
    function do_show() :mixed{
        $this->show();
    }
}
<<__EntryPoint>> function main(): void {
$t = new pass();
$t->do_show();

$t2 = new fail();
$t2->do_show();

echo "Done\n"; // shouldn't be displayed
}
