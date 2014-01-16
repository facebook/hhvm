<?php

include "queryable.inc";

function from() { return "ok\n"; }

$customers = new Queryable();

$q = from $c in $customers
     from $o in $c->Orders
     select tuple($c->Name, $o->OrderID, $o->Total);

foreach ($q as $e) {
  echo $e."\n";
}

echo from();
