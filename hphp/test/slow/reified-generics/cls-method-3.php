<?hh

class C {
  public static function f<reify T>() :mixed{
    var_dump("hi");
  }
}
<<__EntryPoint>>
function main(): void {
  $c = C::class;
  try {
    $c::f<int>();
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
