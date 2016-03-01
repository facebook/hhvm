<?php
require_once ($GLOBALS["HACKLIB_ROOT"]);
function f() {
  \HH\invariant(1 == 1, "test");
  \HH\invariant_violation("test2");
}
