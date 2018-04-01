<?hh // decl

namespace FOO;

function int() {
  return function () {};
}

function string() {
  return new class{};
}

var_dump(\FOO\int(), \FOO\string());
