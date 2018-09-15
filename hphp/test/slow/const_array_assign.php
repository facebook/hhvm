<?hh // strict

class ConstArrayAssign {
  const array<int> MyData = array(1, 2, 3);

  function assign_my_data(int $x): void {
    self::MyData[0] = $x;
  }
}
