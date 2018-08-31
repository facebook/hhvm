<?php

/* This version of the test documents the behavior before the "additional
 * behavior change" portion of the RFC is implemented (array internal pointer
 * is updated by foreach by reference).
 */
<<__EntryPoint>>
function main_04a_by_reference_ignores_and_undoes_pointer_moves() {
$a = [1,2,3,4];
foreach($a as &$v) {echo "$v - "; next($a); var_dump(current($a));}
}
