<?hh

<<__Memoize>>
function memo(int $x) :mixed{
  var_dump($x);
}

function helper(mixed $f)[ctx $f] :mixed{
  memo(1);
  memo(1);
  memo(1);
}

<<__EntryPoint>>
function main() :mixed{
  helper(() ==> 1);
}
