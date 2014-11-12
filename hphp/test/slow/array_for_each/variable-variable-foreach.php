<?php
$rows=array(1,2,3);


function fn1() {
    global $rows;
    global $row;
    $a="rows";
    $b="row";

    // TYPICAL FOREACH
    foreach($rows as $row) {
        fn2();
    }

    // USING VARIABLE VARIABLE
    foreach($$a as $$b) {
        fn2();
    }

    // THE MALFORMED ARRAY
    foreach($rows as $row) {
        fn2();
    }
}

function fn2() {
    global $row;
    echo "row=${row}\n";
}

// ORIGINAL ARRAY
print_r($rows);

// SOME ITERATIONS
fn1();

// THE MALFORMED ARRAY
print_r($rows);
