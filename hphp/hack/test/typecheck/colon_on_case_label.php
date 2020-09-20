<?hh // partial

function foo($x): void {

  switch ($x) {
    case 1;
      echo "hi";
      break;
    default:
      echo "bye";
  }
}

foo(1);
