<?hh

trait T1 {
  require extends NonExistent;
}

class X {
  use T1;
}
<<__EntryPoint>>
function main_require_constraint_nonexistent_class() :mixed{
  $x = new X();
}
