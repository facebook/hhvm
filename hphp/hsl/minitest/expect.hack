namespace HH\__Private\MiniTest;

function expect<T>(T $x): ExpectObj<T> {
  return new ExpectObj($x);
}
