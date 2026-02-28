<?hh

function foo() :mixed{
  if (mt_rand() == 0) return vec[];
  $z = vec[true, mt_rand() ? true : false];
  return $z;
}

function bar() :mixed{
  $x = foo();
  $y = $x[1];
  for ($i = 0; $i < 10; ++$i) mt_rand();
  return $y;
}


<<__EntryPoint>>
function main_movzbool() :mixed{
for ($i = 0; $i < 100; ++$i) {
  bar();
}
echo "Done\n";
}
