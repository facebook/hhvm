<?php

include "queryable.inc";

class C {
  function join<T>(T $t) { return "ok\n"; }
}

function into() { return (new C())->join(2); }

$customers = new Queryable();

$q = from $c in $customers
     join $o in $orders on $c->CustomerID equals $o->CustomerID into $co
     let $n = count($co)
     where $n >= 10
     select Map {"Name" => $c->Name, "OrderCount" => $n };

foreach ($q as $e) {
  print_result($e);
}

echo into();
