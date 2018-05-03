<?hh // strict

function test(): void {
  switch (true) {
    case true:
      switch (false) {}
      echo $x;
      $x = 1;
      break;
    default:
      $x = 2;
      break;
  }
}
