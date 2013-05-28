<?php

function fruit() {
 $a = 123;
 yield $a;
yield break;
yield ++$a;
}
 foreach (fruit() as $fruit) {
 var_dump($fruit);
}

