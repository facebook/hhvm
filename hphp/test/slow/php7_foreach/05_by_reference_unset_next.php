<?php

<<__EntryPoint>>
function main_05_by_reference_unset_next() {
$a = [1,2,3]; foreach($a as &$v) {echo "$v\n"; unset($a[1]);}
}
