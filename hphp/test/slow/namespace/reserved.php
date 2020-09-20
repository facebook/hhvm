<?hh // decl

namespace FOO;

function int() {
  return function () {};
}

<<__EntryPoint>>
function main_reserved() {
\var_dump(\FOO\int());
}
