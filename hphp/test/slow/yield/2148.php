<?php

function nums() {
 $i = 0;
 while ($i < 3) yield $i++;
}
 foreach (nums() as $num) {
 var_dump($num);
}

