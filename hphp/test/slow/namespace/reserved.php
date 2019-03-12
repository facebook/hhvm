<?hh // decl

namespace FOO;

function int() {
  return function () {};
}

function string() {
  return new class{};
}


<<__EntryPoint>>
function main_reserved() {
\var_dump(\FOO\int(), \FOO\string());
}
