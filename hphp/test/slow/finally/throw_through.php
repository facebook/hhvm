<?php

function foo() {
  try {
    throw new Exception('foo');
  } finally {
    var_dump("me first!");
  }
}
foo();
