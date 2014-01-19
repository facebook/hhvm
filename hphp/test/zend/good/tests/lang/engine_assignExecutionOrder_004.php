<?php

function i1() {
        echo "i1\n";
        return 1;
}

function i2() {
        echo "i2\n";
        return 1;
}

function i3() {
        echo "i3\n";
        return 3;
}

function i4() {
        global $a;
        $a = array(10, 11, 12, 13, 14);
        echo "i4\n";
        return 4;
}

$a = 0; // $a should not be indexable till the i4 has been executed
list($a[i1()+i2()], , list($a[i3()], $a[i4()]), $a[]) = array (0, 1, array(30, 40), 3, 4);

var_dump($a);

?>