<?php

function & genRefInner() {
    $var = 1;
    yield $var;
}

function & genRefOuter() {
    return genRefInner();
}

foreach(genRefOuter() as $i) {
    var_dump($i);
}

?>
