<?hh

class father {
    function f0() :mixed{}
    function f1() :mixed{}
    public function f2() :mixed{}
    protected function f3() :mixed{}
    private function f4() :mixed{}
}

class same extends father {

    // overload fn with same visibility
    function f0() :mixed{}
    public function f1() :mixed{}
    public function f2() :mixed{}
    protected function f3() :mixed{}
    private function f4() :mixed{}
}

class fail extends same {
    function f4() :mixed{}
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
