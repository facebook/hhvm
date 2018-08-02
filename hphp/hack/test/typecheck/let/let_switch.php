<?hh // experimental

function foo(int $i): void {
  switch ($i) {
    case 1:
      let ret = 1;
      break;
    case 2:
      let ret = 42;
      break;
    default:
      let ret = -1;
      break;
  }
  echo ret;
}
