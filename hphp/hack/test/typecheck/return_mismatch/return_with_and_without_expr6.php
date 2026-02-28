<?hh

function test(int $x0): mixed {
  return  (int $i) ==> {
    if ($i == 1) {
        return 3;
      } else {
        return;
      }
  };
}
