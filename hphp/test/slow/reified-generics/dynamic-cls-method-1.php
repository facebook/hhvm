<?hh

class C {
  static function f<reify T>($x) :mixed{
    var_dump($x is T);
  }
}

<<__EntryPoint>>
function main() :mixed{
  $c = 'C';
  try { $c::f(1); } catch (Exception $_) { echo "caught!\n"; }
  $c::f<int>(1);
  $c::f<int>("hi");
}
