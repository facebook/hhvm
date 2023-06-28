<?hh

class X { static function Y() :mixed{} }

<<__EntryPoint>>
function main() :mixed{
  $c = X::Y<>;
  $d = __hhvm_intrinsics\launder_value($c);

  echo json_encode($c)."\n";
  echo json_encode($d, JSON_PRETTY_PRINT)."\n";
  echo json_encode($d, JSON_FB_LOOSE)."\n";
  echo json_encode($d, JSON_FB_FORCE_PHP_ARRAYS)."\n";
  echo json_encode($d, JSON_FORCE_OBJECT)."\n";
}
