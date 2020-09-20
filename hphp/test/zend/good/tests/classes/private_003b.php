<?hh

class pass {
    private function show() {
        echo "Call show()\n";
    }

    protected function good() {
        $this->show();
    }
}

class fail extends pass {
    public function ok() {
        $this->good();
    }

    public function not_ok() {
        $this->show();
    }
}
<<__EntryPoint>> function main(): void {
$t = new fail();
$t->ok();
$t->not_ok(); // calling a private function

echo "Done\n"; // shouldn't be displayed
}
