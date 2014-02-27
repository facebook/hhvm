<?php

include "queryable.inc";

$customers = new Queryable();
$name = "foo";
$id = 123;

$q = from $c in $customers
     from $o in $c->Orders
     where $c->Name == $name && $id == $o->OrderId
     select tuple($c->Name, $o->OrderID, $o->Total);

foreach ($q as $e) {
  print_result($e);
}
