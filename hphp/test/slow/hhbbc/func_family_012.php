<?php

class Base {
  function asd() { echo "Base\n"; return 1; }
}

class Obj extends Base {
  function asd() { echo "Derived\n"; return "12"; }
}

function foo(Base $x, Obj $y) {
  $b = true;
loop:
  if (!$b) goto done;
  $x = $b ? $x : $y;
  $foo = $x->asd();
  $b = false;
  goto loop;
done:
  var_dump($foo);
}

foo(new Base, new Obj);
foo(new Obj, new Obj);
