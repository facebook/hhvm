<?php

<<__EntryPoint>>
function main_06_by_reference_add_element() {
$a = [1,2]; foreach($a as &$v) {echo "$v\n"; $a[2]=3;}
}
