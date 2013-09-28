<?php

function g($x) {
  var_dump($x);
  if ($x == 123) {
    throw new Exception();
  }
}
function f($x) {
  if ($x == 1) {
    goto mylabel1;
  }
 else if ($x == 2) {
    goto mylabel2;
  }
 else if ($x == 3) {
    goto mylabel3;
  }
 else if ($x == 4) {
    goto mylabel4;
  }
  try {
    g($x);
    echo "Should not get here\n";
    try {
      try {
        mylabel1:        g($x);
      }
 catch (exception $e) {
        echo "1a\n";
      }
      try {
        mylabel2:        g($x);
      }
 catch (exception $e) {
        echo "1b\n";
      }
    }
 catch (exception $e) {
      echo "1\n";
    }
    try {
      try {
        mylabel3:        g($x);
      }
 catch (exception $e) {
        echo "2a\n";
      }
      try {
        mylabel4:        g($x);
      }
 catch (exception $e) {
        echo "2b\n";
      }
    }
 catch (exception $e) {
      echo "2\n";
    }
  }
 catch (Exception $e) {
    echo "0\n";
  }
}
f(1);
f(2);
f(3);
f(4);
f(123);
