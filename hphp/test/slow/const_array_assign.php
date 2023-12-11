<?hh

class ConstArrayAssign {
  const varray<int> MyData = vec[1, 2, 3];

  function assign_my_data(int $x): void {
    self::MyData[0] = $x;
  }
}

