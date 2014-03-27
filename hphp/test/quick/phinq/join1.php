<?php

include "queryable.inc";

class C {
  function join() { return "ok\n"; }
}

$customers = new Queryable();

$q = from $c in $customers
     join $o in $orders on $c->CustomerID equals $o->CustomerID
     select tuple($c->Name, $o->OrderID, $o->Total);

foreach ($q as $e) {
  print_result($e);
}

echo (new C())->join();
