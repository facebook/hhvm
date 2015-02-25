<?php


function callee(HH\Map &$c) {

}

function main() {
  $c = HH\Map {};
  callee($c);
  var_dump($c);
}

main();
