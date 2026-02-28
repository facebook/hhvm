<?hh

class C {}

function reified_and_variadic<reify T>(...$vs) :mixed{
  foreach ($vs as $v) {
    var_dump($v);
  }
}

<<__EntryPoint>>
function main() :mixed{
  reified_and_variadic<bool>(new C(), new C());
}
