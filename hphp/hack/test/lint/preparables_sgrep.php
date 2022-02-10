<?hh

function test($x, $y, $z) {
  gena(varray[A]);
  genak(A, new ImmSet($x));
  genak(A, B->toArray());
  list($a, $b) = await gena(varray[$x, $y]);
  list($a, $b, $c) = await gena(varray[$x, $y, $z]);
  list($a, $b) = Asio::awaitSynchronously(gena(varray[$x, $y]));
  list($a, $b, $c) = Asio::awaitSynchronously(gena(varray[$x, $y, $z]));

  return Asio::awaitSynchronously(A);
}
