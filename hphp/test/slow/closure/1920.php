<?php

$abc = 123;
 $a = function () use ($abc) {
 var_dump($abc);
}
;
 $a();
