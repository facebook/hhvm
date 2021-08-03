<?hh

<<__EntryPoint>> function main(): void {
  // Precision loss when comparing big ints w/ floats.
  var_dump(HH\Lib\Legacy_FIXME\eq((1 << 60) + 1, (float)(1 << 60)));
  var_dump(HH\Lib\Legacy_FIXME\eq((1 << 60), (float)(1 << 60)));

  var_dump(-0.0 == 0.0);
  var_dump(HH\Lib\Legacy_FIXME\eq(-0.0, 0));
  var_dump(0.0001 == 0);
  var_dump(-0.0001 == 0);

  var_dump(-0.0 != 0.0);
  var_dump(HH\Lib\Legacy_FIXME\neq(-0.0, 0));
  var_dump(0.0001 != 0);
  var_dump(-0.0001 != 0);

  $nan = acos(1.01);
  var_dump($nan == $nan);
  var_dump($nan != $nan);
  var_dump($nan == 0.0);
  var_dump($nan != 0.0);
  var_dump($nan === $nan);
  var_dump($nan !== $nan);
}
