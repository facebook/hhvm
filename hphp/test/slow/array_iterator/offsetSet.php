<?php
    $it = new ArrayIterator();
    $it[] = "foo";
    $it[] = "bar";
    $it[] = "baz";

    foreach($it as $value) {
        print_r($value);
    }

    print_r(sizeof($it));
?>

