<?php
function create_references(&$array) {
    foreach ($array as $key => $value) {
        create_references($array[$key]);
    }
}  

function change_copy($copy) {
        $copy['b']['z']['z'] = $copy['b'];
}  

$data = [
    'a' => [
        'b' => [],
    ], 
]; 
 
create_references($data);
 
$copy = $data['a'];
var_dump($copy);

change_copy($copy);
var_dump($copy); //RECURSION
?>
