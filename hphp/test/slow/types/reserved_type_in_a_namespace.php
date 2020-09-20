<?hh
namespace foo;

class int {}

function bar(int $int) {
  \var_dump($int);
}

<<__EntryPoint>>
function main() {
  bar(new int());
  bar(42);
}
