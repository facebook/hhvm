<?php

include "queryable.inc";

$customers = new Queryable();

$q = from $c in $customers
     join $o in $orders on $c->CustomerID equals $o->CustomerID
     join $d in $details on $o->OrderID equals $d->OrderID
     join $p in $products on $d->ProductsID equals $p->ProductID
     select tuple($c->Name, $o->OrderDate, $p->ProductName);

foreach ($q as $e) {
  print_result($e);
}
