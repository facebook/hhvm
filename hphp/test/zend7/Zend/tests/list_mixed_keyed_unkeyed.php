<?php

$contrivedKeyedAndUnkeyedArrayExample = [
    0,
    1 => 1,
    "foo" => "bar"
];

list($zero, 1 => $one, "foo" => $foo) = $contrivedKeyedAndUnkeyedArrayExample;

?>
