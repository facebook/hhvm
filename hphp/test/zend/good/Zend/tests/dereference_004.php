<?hh

class foo {
    public $x = varray[];
    public function __construct() {
        $h = varray[];
        $h[] = new stdClass;
        $this->x = $h;
    }
    public function __invoke() {
        return $this->x;
    }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$fo = new foo;
var_dump($fo()[0]);
}
