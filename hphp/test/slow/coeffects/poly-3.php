<?hh

function f1(mixed $a)[$a::C] :mixed{
  echo "in f1\n";
}

<<__EntryPoint>>
function main() :mixed{
  f1(1);
}
