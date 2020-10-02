<?hh // partial

function test1(varray<string> $xs): varray<string> {
  return array_values($xs);
}

function test2(darray<int, string> $xs): varray<string> {
  return array_values($xs);
}

function test3(darray<string, string> $xs): varray<string> {
  return array_values($xs);
}

function test4(ConstSet<int> $xs): varray<int> {
  return array_values($xs);
}

function test5(ConstMap<string, int> $xs): varray<int> {
  return array_values($xs);
}

function test6(ConstVector<int> $xs): varray<int> {
  return array_values($xs);
}
