<?hh

namespace A;

function invariant() {
  \var_dump('invariant');
}

<<__EntryPoint>>
function main_invariant_namespace() {
\A\invariant();
}
