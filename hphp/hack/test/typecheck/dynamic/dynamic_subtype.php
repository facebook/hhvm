<?hh

async function test(dynamic $x): Awaitable<int> {
  return $x; // error, $x does not subtype with int
}
