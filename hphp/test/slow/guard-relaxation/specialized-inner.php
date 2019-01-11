<?php


function callee(HH\Map &$c) {

}

function main() {
  $c = HH\Map {};
  callee(&$c);
  var_dump($c);
}


<<__EntryPoint>>
function main_specialized_inner() {
main();
}
