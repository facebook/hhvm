<?php

$array1 = array(
       'friends' => 5,
       'children' => array(
               'dogs' => 0,
       ),
);

$array2 = array(
       'friends' => 10,
       'children' => array(
               'cats' => 5,
       ),
);

$merged = array_merge_recursive($array1, $array2);

var_dump($array1, $array2);

?>