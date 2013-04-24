<?php
$obj = new SplFixedArray(2);
var_dump(count($obj));
class SplFixedArray2 extends SplFixedArray {
    public function count() {
        return -parent::count();
    }
}
$obj = new SplFixedArray2(2);
var_dump(count($obj));
?>