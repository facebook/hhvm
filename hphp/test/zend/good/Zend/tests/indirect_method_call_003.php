<?hh

class foo {
    public $x = 1;

    public function getX() :mixed{
        return $this->x;
    }
    public function setX($val) :mixed{
        $this->x = $val;
        return $this;
    }
}
<<__EntryPoint>> function main(): void {
$X = (new foo)->setX(10)->getX();
var_dump($X); // int(10)
}
