<?php

function test($a){
  echo "$a\n";
}
test(1, test(2), test(3, test(4), test(5)));
