<?php

<<__EntryPoint>>
function main_02_by_value_ignores_modifications() {
$a = [1,2,3]; $b = &$a; foreach($a as $v) {echo "$v\n"; unset($a[1]);}
}
