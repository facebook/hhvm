<?hh

class C<reify T1, reify T2> {}

class D extends C<int> {
  function f() :mixed{ new parent(); }
}

<<__EntryPoint>>
function main() :mixed{
  try {
    (new D)->f();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
