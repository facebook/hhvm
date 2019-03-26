<?php

function g($x) {
  $x[0][1] = 10;
  var_dump($x[0][2]);
}

<<__EntryPoint>>
function main_536() {
g(array());
g(array(0));
}
