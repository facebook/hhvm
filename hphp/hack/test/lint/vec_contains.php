<?hh

function test1(Vector<string> $vs):bool {
  return $vs->containsKey(3);
}


function test2(Vector<string> $vs):int {
  return $vs->linearSearch(null);
}
