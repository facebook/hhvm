<?hh

class foo {
    public $x = vec[];
    public function __construct() {
        $h = vec[];
        $h[] = new stdClass;
        $this->x = $h;
    }
    public function __invoke() :mixed{
        return $this->x;
    }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$fo = new foo;
var_dump($fo()[0]);
}
