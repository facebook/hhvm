<?php

include "queryable.inc";

function group() { return "ok\n"; }
function by() { return group(); }
function into() { return by(); }

$customers = new Queryable();

$q = from $c in $customers
     group $c by $c->Country into $g
     select Map { "Country" => $g->Key, "CustCount" => $g->Count() };

foreach ($q as $e) {
  echo $e."\n";
}

echo into();
