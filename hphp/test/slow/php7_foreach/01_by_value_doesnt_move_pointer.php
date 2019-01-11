<?php

<<__EntryPoint>>
function main_01_by_value_doesnt_move_pointer() {
$a = [1,2,3]; foreach($a as $v) {echo $v . " - " . current(&$a) . "\n";}
}
