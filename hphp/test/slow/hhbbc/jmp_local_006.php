<?hh

function main(varray $x = null) :mixed{
  if ($x) {
    echo is_array($x);
    echo "\n";
  } else {
    echo "empty or not array\n";
  }
}


<<__EntryPoint>>
function main_jmp_local_006() :mixed{
main(vec[]);
main(vec[1,2,3]);
}
