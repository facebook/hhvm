<?php

<<__EntryPoint>>
function main_10_by_reference_array_pop() {
$a=[1,2,3,4]; foreach($a as &$v) { echo "$v\n"; array_pop($a);}
}
