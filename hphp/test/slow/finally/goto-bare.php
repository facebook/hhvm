<?php

function foo() {
  try {
    goto label;
  } finally {
    var_dump("finally1");
  }

  return "wrong!";

label:
  var_dump("label");
  return "return2";
}

var_dump(foo());
