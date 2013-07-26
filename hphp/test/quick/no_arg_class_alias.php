<?php
function f() {
  // try not to segv when you have no arguments:
  class_alias();
}
f();

