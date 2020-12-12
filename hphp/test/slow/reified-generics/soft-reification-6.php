<?hh

function f<<<__Soft>> reify T>() {
  echo "ok\n";
}

class C {
  function f<<<__Soft>> reify T>() {
    echo "ok\n";
  }
  static function fs<<<__Soft>> reify T>() {
    echo "ok\n";
  }
}


<<__EntryPoint>>
function main() {
  $x = 'f';
  $x();
  C::fs();
  (new C())->f();
}
