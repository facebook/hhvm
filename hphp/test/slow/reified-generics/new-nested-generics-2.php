<?hh

class C<reify T> {
  function g() :mixed{
    echo "yes\n";
  }
}

function f<reify T>() :mixed{
  $x = new T();
  $x->g();
  $y = new T();
  $y->g();
}

<<__EntryPoint>>
function main() :mixed{
  f<C<int>>();
}
