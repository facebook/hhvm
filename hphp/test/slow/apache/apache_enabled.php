<?php

function test() {
  if (!function_exists('apache_note')) {
    return false;
  }
  return "YES";
}

var_dump(test());
