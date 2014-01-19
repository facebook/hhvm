<?php

include "queryable.inc";

$customers = new Queryable();

$q = from $c in $customers
     where $c->City === "London"
     select $c;

foreach ($q as $e) {
  echo $e."\n";
}
