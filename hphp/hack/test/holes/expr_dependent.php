<?hh

abstract class WithTypeConstant {
  abstract const type TX;

  abstract public function gen(): this::TX;
  abstract public function log(this::TX $x): void;
}

function my_gen<T as WithTypeConstant, TR>(T $left): TR where TR = T::TX {
  return $left->gen();
}

function my_log<T as WithTypeConstant, TR>(T $left): (function(
  TR,
): void) where TR = T::TX {
  return (TR $x) ==> ($left->log($x));
}

function my_error(WithTypeConstant $x, WithTypeConstant $y): void {
  $gen = my_gen($x);
  $log = my_log($y);
  /* HH_FIXME[4110] we need expression-dependent types for Hole reporting here
     or we end up with `WithTypeConstant::TX` for both source and destination
     type */
  $log($gen);
}
