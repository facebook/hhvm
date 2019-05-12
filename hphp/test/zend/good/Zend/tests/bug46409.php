<?php
class Callback {
    protected $val = 'hello, world';

    public function __invoke() {
        return $this->val;
    }
}
<<__EntryPoint>> function main() {
$cb = new Callback();
echo $cb(),"\n";
$a = array(1, 2);
$b = array_map($cb, $a);
print_r($b);
}
