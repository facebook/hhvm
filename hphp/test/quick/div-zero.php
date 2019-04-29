<?hh


function main($num, $zero) {
  try {
    $z = $num / 0;
  } catch (DivisionByZeroException $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    $zz = $num / $zero;
  } catch (DivisionByZeroException $e) {
    echo $e->getMessage(), "\n";
  }
}
main(123, 0);
