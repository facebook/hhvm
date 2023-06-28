<?hh

namespace A;

function invariant() :mixed{
  \var_dump('invariant');
}

<<__EntryPoint>>
function main_invariant_namespace() :mixed{
\A\invariant();
}
