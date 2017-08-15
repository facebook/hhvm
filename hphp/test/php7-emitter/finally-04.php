<?php

function foo($x) {
  try {
    try {
      try {
        return $x;
      } finally {
        var_dump("foo");
      }
    } finally {
      return 0;
    }
  } finally {
    var_dump("bar");
  }
  return 20;
}

var_dump(foo(true));
var_dump(foo(26));
