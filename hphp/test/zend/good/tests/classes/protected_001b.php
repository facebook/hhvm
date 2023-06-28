<?hh

class pass {
    protected function fail() :mixed{
        echo "Call fail()\n";
    }

    public function good() :mixed{
        $this->fail();
    }
}
<<__EntryPoint>> function main(): void {
$t = new pass();
$t->good();
$t->fail();// must fail because we are calling from outside of class pass

echo "Done\n"; // shouldn't be displayed
}
