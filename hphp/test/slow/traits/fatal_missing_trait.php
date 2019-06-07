<?hh

if (isset($g)) {
  include 'fatal_missing_trait.inc';
}

class C {
  use T;
}

var_dump(new C());
