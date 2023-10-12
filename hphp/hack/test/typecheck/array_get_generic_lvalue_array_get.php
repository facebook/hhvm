<?hh //strict

/**
 * Assignment to array makes it unsafe to return it as original generic type
 */
function f<Tv as darray<mixed, mixed>>(Tv $a): Tv {
  $a['a'] = 4;
  return $a;
}

function test(darray<string, string> $a): darray<string, string> {
  return f($a);
}
