<?php

echo "\n*** Testing array_keys() on various arrays ***";
$arrays = array(
  array(), 
  array(0),
  array( array() ), 
  array("Hello" => "World"), 
  array("" => ""),  
  array(1,2,3, "d" => array(4,6, "d")),
  array("a" => 1, "b" => 2, "c" =>3, "d" => array()),
  array(0 => 0, 1 => 1, 2 => 2, 3 => 3),  
  array(0.001=>3.000, 1.002=>2, 1.999=>3, "a"=>3, 3=>5, "5"=>3.000),
  array(TRUE => TRUE, FALSE => FALSE, NULL => NULL, "\x000", "\000"),
  array("a" => "abcd", "a" => "", "ab" => -6, "cd" => -0.5 ),
  array(0 => array(), 1=> array(0), 2 => array(1), ""=> array(),""=>"" )
);

$i = 0;
/* loop through to test array_keys() with different arrays */
foreach ($arrays as $array) {
  echo "\n-- Iteration $i --\n";
  var_dump(array_keys($array)); 
  $i++;
}

echo "Done\n";
?>