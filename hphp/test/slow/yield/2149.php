<?php

function nums() {
 $i = 0;
 do yield $i++;
 while ($i < 3);
}


 <<__EntryPoint>>
function main_2149() {
foreach (nums() as $num) {
 var_dump($num);
}
}
