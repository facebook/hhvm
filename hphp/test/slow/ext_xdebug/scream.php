<?php

ini_set("xdebug.scream", 1);

function foo() {
  trigger_error("Test warning", E_USER_WARNING);
}

@foo();
