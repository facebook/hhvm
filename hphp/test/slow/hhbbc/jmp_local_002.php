<?hh

function main(int $x) :mixed{
  if (!$x) {
    echo $x;
  }
}


<<__EntryPoint>>
function main_jmp_local_002() :mixed{
main(0);
}
