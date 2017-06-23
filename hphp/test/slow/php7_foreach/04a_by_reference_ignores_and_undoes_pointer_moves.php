<?php
/* This version of the test documents the behavior before the "additional
 * behavior change" portion of the RFC is implemented (array internal pointer
 * is updated by foreach by reference).
 */
$a = [1,2,3,4];
foreach($a as &$v) {echo "$v - "; next($a); var_dump(current($a));}
