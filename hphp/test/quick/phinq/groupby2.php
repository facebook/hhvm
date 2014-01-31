<?php

include "queryable.inc";

function in() { return "ok\n"; }

$customers = new Queryable();

$q = from $c in $customers
     group $c->Name by $c->Country;

foreach ($q as $e) {
  print_result($e);
}

echo in();
