<?hh

class pass {
    private function show() :mixed{
        echo "Call show()\n";
    }

    protected function good() :mixed{
        $this->show();
    }
}

class fail extends pass {
    public function ok() :mixed{
        $this->good();
    }

    public function not_ok() :mixed{
        $this->show();
    }
}
<<__EntryPoint>> function main(): void {
$t = new fail();
$t->ok();
$t->not_ok(); // calling a private function

echo "Done\n"; // shouldn't be displayed
}
