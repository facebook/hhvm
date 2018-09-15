<?php

function h() {
  return array_filter(array(1, 2, 3),
                      function($e) {
 return !($e & 1);
 }
);
}

<<__EntryPoint>>
function main_1929() {
h();
var_dump(h());
}
