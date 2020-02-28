<?hh

function main(array $x = null) {
  if ($x) {
    echo is_array($x);
    echo "\n";
  } else {
    echo "empty or not array\n";
  }
}


<<__EntryPoint>>
function main_jmp_local_006() {
main(varray[]);
main(varray[1,2,3]);
}
