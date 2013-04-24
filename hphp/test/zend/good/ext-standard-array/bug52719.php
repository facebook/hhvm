<?php
$array = array("hello", array("world"));
$userdata = array();
array_walk_recursive(
    $array,
    function ($value, $key, &$userdata) { },
    $userdata
);
echo "Done";
?>