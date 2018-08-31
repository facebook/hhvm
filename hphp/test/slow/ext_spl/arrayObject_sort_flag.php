<?php


<<__EntryPoint>>
function main_array_object_sort_flag() {
$ao = new ArrayObject([0=>'c', '1a'=>3, 'c'=>'a', 'b'=>'b']);

print_r($ao);
$ao->asort();
print_r($ao);
$ao->asort(SORT_STRING);
print_r($ao);
}
