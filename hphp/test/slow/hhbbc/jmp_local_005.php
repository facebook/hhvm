<?hh

function main(?string $x = null) :mixed{
  if (!$x) {
    var_dump($x);
  }
}


<<__EntryPoint>>
function main_jmp_local_005() :mixed{
main("");
main("0");
}
