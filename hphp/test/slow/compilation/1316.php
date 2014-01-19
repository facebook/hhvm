<?php

$x = 1;
switch ($x++ ?: -1) {
}
;
var_dump($x);
