<?hh

namespace FOO;

function int() :mixed{
  return function () {};
}

<<__EntryPoint>>
function main_reserved() :mixed{
\var_dump(\FOO\int());
}
