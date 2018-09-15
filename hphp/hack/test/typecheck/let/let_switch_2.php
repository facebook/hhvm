<?hh // experimental

function foo(int $i): void {
  switch ($i) {
    case 1:
      let ret = 42;
      // FALLTHROUGH
    case 2:
      echo ret; // error
      break;
    default:
      break;
  }
}
