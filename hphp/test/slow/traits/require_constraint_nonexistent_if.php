<?hh

trait T1 {
  require implements NonExistent;
}

class X {
  use T1;
}

<<__EntryPoint>>
function main_require_constraint_nonexistent_if() :mixed{
  $x = new X();
}
