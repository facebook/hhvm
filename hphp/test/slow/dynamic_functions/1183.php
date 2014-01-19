<?php

function test(&$a, $b) {
 $a = 'ok';
}
 $a = 'Test';
 $a($a, 10);
 print $a;
