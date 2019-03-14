<?hh // partial

function darray<Tk, Tv>(mixed $x): darray<Tk, Tv> {
  return (array)$x;
}

function testDarray($x): darray<int, int> {
  return darray($x);
}
