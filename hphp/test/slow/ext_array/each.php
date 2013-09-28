<?php

function a() {
  $foo = array("bob", "fred", "jussi", "jouni", "egon", "marliese");
  $bar = each($foo);
  var_dump($bar);
}

function b() {
  $foo = array("Robert" => "Bob", "Seppo" => "Sepi");
  $bar = each($foo);
  var_dump($bar);
}

function c() {
  $fruit = array("a" => "apple", "b" => "banana", "c" => "cranberry");
  reset($fruit);
  $output = '';
  while (true) {
    $item = each($fruit);
    if ($item === false) break;
    $output .= $item[0];
    $output .= ": ";
    $output .= $item[1];
    $output .= "\n";
  }
  var_dump($output);
}

a();
b();
c();
