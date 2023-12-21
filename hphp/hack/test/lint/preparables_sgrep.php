<?hh

function test($x, $y, $z) {
  gena(vec[A]);
  genak(A, new ImmSet($x));
  genak(A, B->toArray());
  list($a, $b) = await gena(vec[$x, $y]);
  list($a, $b, $c) = await gena(vec[$x, $y, $z]);
  list($a, $b) = Asio::awaitSynchronously(gena(vec[$x, $y]));
  list($a, $b, $c) = Asio::awaitSynchronously(gena(vec[$x, $y, $z]));

  return Asio::awaitSynchronously(A);
}
