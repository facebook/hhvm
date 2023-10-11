<?hh

/**
 * Generic type remains generic if it was not modified.
 */
function f<Tv as KeyedContainer<string, mixed>>(Tv $a): Tv {
  $a['a'];
  return $a;
}

function test(darray<string, string> $a): darray<string, string> {
  return f($a);
}
