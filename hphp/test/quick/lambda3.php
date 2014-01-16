<?hh

function main() {
  $capture1 = "yo";
  $get_func = ($x, $y) ==> {
    return $more ==> $x . $y . $capture1 . $more;
  };

  $f = $get_func("one ", "two ");
  echo $f("\n");
}

main();
