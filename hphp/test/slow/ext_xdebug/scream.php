<?php

ini_set("hhvm.xdebug-not-done.scream", 1);

function foo() {
  trigger_error("Test warning", E_USER_WARNING);
}

@foo();
