<?hh

function g() :mixed{
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

  !(c::$x ?? false);
  !(c::$x[0] ?? false);
  !(c::$x->prop[0] ?? false);

  f(c::$x);
  f(c::$x[0]);
  f(c::$x->prop[0]);
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
