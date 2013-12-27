<?php

include "queryable.inc";

$customers = new Queryable();

$q = from $c in $customers
     join $c in $orders on $c->CustomerID equals $o->CustomerID
     join $d in $details on $o->OrderID equals $d->OrderID
     join $p in $products on $d->ProductsID equals $p->ProductID
     select tuple($c->Name, $o->OrderDate, $p->ProductName);

foreach ($q as $e) {
  echo $e."\n";
}
