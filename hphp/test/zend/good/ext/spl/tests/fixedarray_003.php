<?php
class A extends SplFixedArray {

    public function current() {
        echo "A::current\n";
        return parent::current();
    }
    public function key() {
        echo "A::key\n";
        return parent::key();
    }
    public function rewind() {
        echo "A::rewind\n";
        return parent::rewind();
    }
    public function valid() {
        echo "A::valid\n";
        return parent::valid();
    }
    public function next() {
        echo "A::next\n";
        return parent::next();
    }
}

echo "==Direct instance==\n";
$a = new SplFixedArray(5);
$a[0] = "a";
$a[1] = "c";
$a[2] = "d";
$a[3] = "e";
$a[4] = "f";
foreach ($a as $k => $v) {
    echo "$k => $v\n";
}
echo "==Child instance==\n";
$a = new A(5);
$a[0] = "a";
$a[1] = "c";
$a[2] = "d";
$a[3] = "e";
$a[4] = "f";
foreach ($a as $k => $v) {
    echo "$k => $v\n";
}
?>
===DONE===