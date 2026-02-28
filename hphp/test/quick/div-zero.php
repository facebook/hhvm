<?hh


function main($num, $zero) :mixed{
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
<<__EntryPoint>> function main_entry(): void {
main(123, 0);
}
