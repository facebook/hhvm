<?hh
namespace foo;

class int {}

function bar(int $int) :mixed{
  \var_dump($int);
}

<<__EntryPoint>>
function main() :mixed{
  bar(new int());
  bar(42);
}
