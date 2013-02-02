<?php

function foo($x) {
  var_dump((int)$x);
}

foo("1234");
foo("1234.56");
foo("123456789012345");
foo("123e4");
foo("123456789123456789123456789");
foo("-65");
foo("");
foo("q123");

var_dump((int)"123456789012345");
var_dump((int)"1234.56");
var_dump((int)"1234");
var_dump((int)"123e4");
var_dump((int)"123456789123456789123456789");
var_dump((int)"-65");
var_dump((int)"");
var_dump((int)"q123");

