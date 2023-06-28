<?hh

function f<<<__Soft>> reify T>() :mixed{
  echo "ok\n";
}

class C {
  function f<<<__Soft>> reify T>() :mixed{
    echo "ok\n";
  }
  static function fs<<<__Soft>> reify T>() :mixed{
    echo "ok\n";
  }
}


<<__EntryPoint>>
function main() :mixed{
  $x = 'f';
  $x();
  C::fs();
  (new C())->f();
}
