<?php

function bug1($a, $b) {
foreach ($b[$a++ + $a++] as &$x) {
 echo $x;
 }
}
