<?hh

class foo {
    public $x = varray[2];

    public function __call($x, $y) {
        if (count($this->x) == 1) {
            $this->x[] = $y[0];
        }
        return $this->x;
    }
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

$foo = new foo;

$x = varray[1];

$foo->b($x)[1] = 3;

var_dump($foo->b()[0]);
var_dump($foo->b()[1]);
try { var_dump($foo->b()[2]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
