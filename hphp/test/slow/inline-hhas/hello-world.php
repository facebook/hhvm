<?php

function hello() {
  hh\asm('
    String "Hello World"
    RetC
  ');
}

var_dump(hello());
