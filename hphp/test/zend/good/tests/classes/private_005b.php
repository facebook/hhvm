<?hh

class pass {
    private function show() {
        echo "Call show()\n";
    }

    public function do_show() {
        $this->show();
    }
}

class fail extends pass {
    function do_show() {
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
