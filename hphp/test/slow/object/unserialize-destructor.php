<?hh

class Dtor {
  function __destruct() {
    echo "fail!\n";
  }
}

function main() {
  $d = unserialize('O:4:"Dtor":0:{}');
}
main();
