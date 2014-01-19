<?php

function fruit() {
 echo "sadpanda, no fruit";
 yield break;
 }
 foreach (fruit() as $fruit) {
 var_dump($fruit);
}

