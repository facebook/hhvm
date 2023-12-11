<?hh


<<__EntryPoint>>
function main_1933() :mixed{
$my_array = vec[7, 1, 5, 6];
$some_value = 6;
usort(inout $my_array,   function($a, $b) use ($some_value) {
    if ($a === $some_value) {
      if ($b === $some_value) {
        return 0;
      }
 else {
        return -1;
      }
    }
 else if ($b === $some_value) {
      return 1;
    }
    if ($a < $b) return -1;
    if ($a <= $b) return 0;
    return 1;
  }
);
}
