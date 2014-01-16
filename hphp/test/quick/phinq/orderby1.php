<?php

include "queryable.inc";

function orderby() { return "ok\n"; }
function descending() { return orderby(); }
function ascending() { return descending(); }

$customers = new Queryable();

$q = from $c in $customers
     from $o in $c->Orders
     orderby $o->Total descending
     select tuple($c->Name, $o->OrderID, $o->Total);

foreach ($q as $e) {
  echo $e."\n";
}
echo $q."\n";

echo ascending();
