<?hh

function test(KeyedContainer<string, int> $xs): varray<int> {
  return array_values($xs);
}
