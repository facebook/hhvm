<?hh

function foo() :mixed{
  if (mt_rand(0,1)) {
    return dict['foo' => vec[4], 'bar' => 5];
  }
  return dict['bar' => 7];
}

<<__EntryPoint>>
function main() :mixed{
  for ($i = 0; $i < 100; ++$i) {
    try {
      foo()['foo'][0] = 3;
    } catch (OutOfBoundsException $e) {}
  }

  echo "Done.";
}
