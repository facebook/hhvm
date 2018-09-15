<?php

<<__EntryPoint>>
function main_07_by_reference_add_element_while_at_end() {
$a = [1]; foreach($a as &$v) {echo "$v\n"; $a[1]=2;}
}
