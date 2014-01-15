<?php

include "queryable.inc";

function where() { return "ok\n"; }

function foo() {
  $customers = new Queryable();
  return
     from $c in $customers
     where $c->City === "London"
     select $c;
}

$q = foo();
foreach ($q as $e) {
  echo $e."\n";
}

echo where();
