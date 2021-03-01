<?hh

function f1(mixed $a)[$a::C] {
  echo "in f1\n";
}

<<__EntryPoint>>
function main() {
  f1(1);
}
