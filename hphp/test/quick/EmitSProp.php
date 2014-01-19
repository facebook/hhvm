<?php

function g() {
  $c::$x;
  $c::$x[0];
  $c::$x->prop[0];

  $y = $c::$x;
  $y = $c::$x[0];
  $y = $c::$x->prop[0];

  $c::$x = 1;
  $c::$x[0] = 1;
  $c::$x->prop[0] = 1;

  c::$x++;
  c::$x[0]++;
  c::$x->prop[0]++;

  ++c::$x;
  ++c::$x[0];
  ++c::$x->prop[0];

  c::$x += 1;
  c::$x[0] += 1;
  c::$x->prop[0] += 1;

  isset(c::$x);
  isset(c::$x[0]);
  isset(c::$x->prop[0]);

  empty(c::$x);
  empty(c::$x[0]);
  empty(c::$x->prop[0]);

  f(c::$x);
  f(c::$x[0]);
  f(c::$x->prop[0]);

  $c::$a =& $x;
  $c::$a[0] =& $x;
  $c::$a->prop[0] =& $x;

  $x =& $c::$a;
  $x =& $c::$a[0];
  $x =& $c::$a->prop[0];
}

