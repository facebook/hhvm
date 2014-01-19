<?php

function nums() {
 $i = 0;
 do yield $i++;
 while ($i < 3);
}
 foreach (nums() as $num) {
 var_dump($num);
}

