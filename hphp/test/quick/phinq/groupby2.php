<?php

include "queryable.inc";

function in() { return "ok\n"; }

$customers = new Queryable();

$q = from $c in $customers
     group $c->Name by $c->Country;

foreach ($q as $e) {
  echo $e."\n";
}

echo in();
