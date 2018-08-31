<?php

function hello() {
  hh\asm('
    String "Hello World"
    RetC
  ');
}


<<__EntryPoint>>
function main_hello_world() {
var_dump(hello());
}
