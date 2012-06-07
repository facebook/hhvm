<?php

function foo() {
  $bar = $GLOBALS['asd'];
}
foo();
foreach ($GLOBALS as $k => $v) { echo "$k->$v\n"; }
