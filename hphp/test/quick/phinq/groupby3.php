<?php

include "queryable.inc";

function getCustomers() {
  return new Queryable();
}

$c = 1;
$g = 2;

$q = from $c in getCustomers()
     where $c->Id == $g + 1
     group $c by $c->Country into $g
     select Map { "Country" => $g->Key, "CustCount" => $g->Count() };

foreach ($q as $e) {
  print_result($e);
}
