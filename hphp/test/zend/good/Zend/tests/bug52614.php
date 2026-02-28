<?hh
class foo {
    public $a1;
    public $a2 = dict[];
    public $a3;
    public $o2;

    public function f1() :mixed{
        return $this->a1;
    }

    public function f2() :mixed{
        return $this->a2;
    }

    public function f3() :mixed{
        $this->a3 = dict[];
        return $this->a3;
    }

    public function f5() :mixed{
        $this->o2 = new stdClass;
        return $this->o2;
    }





}
<<__EntryPoint>> function main(): void {
$foo = new foo;

$foo->f2()[0] = 1;
var_dump($foo->a2);

$foo->f3()[0] = 1;
var_dump($foo->a3);

$foo->f5()->a = 1;
var_dump($foo->o2);

$foo->a1 = vec[2];

var_dump($foo->a1[0]);
$foo->f1()[0]++;
var_dump($foo->a1[0]);
$foo->a1[0]++;
var_dump($foo->a1[0]);
}
