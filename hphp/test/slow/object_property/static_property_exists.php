<?hh

class C {
    static protected $test = 'foo';
    public function test() {
        var_dump(property_exists($this, 'test'));
    }
}


<<__EntryPoint>>
function main_static_property_exists() {
$c = new C;
$c->test();
}
