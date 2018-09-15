<?php

/* This version of the test documents the behavior before the "additional
 * behavior change" portion of the RFC is implemented (array internal pointer
 * is updated by foreach by reference).
 */
<<__EntryPoint>>
function main_03a_by_reference_moves_pointer() {
$a = [1,2,3]; foreach($a as &$v) {echo $v . " - " . current($a) . "\n"; }
}
