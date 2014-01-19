<?php

function nums() {
 for ($i = 0;
 $i < 3;
 $i++) yield $i;
}
 foreach (nums() as $num) {
 var_dump($num);
}

