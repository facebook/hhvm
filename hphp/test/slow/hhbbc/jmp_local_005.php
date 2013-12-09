<?hh

function main(string $x = null) {
  if (!$x) {
    var_dump($x);
  }
}

main("");
main("0");
