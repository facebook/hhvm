<?php

include "queryable.inc";

function select() { return "ok\n"; }

function foo() {
  $customers = new Queryable();
  return
     from $c in $customers
     select $c;
}

$q = foo();
foreach ($q as $e) {
  echo $e."\n";
}
echo $q."\n";

echo select();
