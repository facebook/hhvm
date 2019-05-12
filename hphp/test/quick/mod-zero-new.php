<?hh

function main($num,$zero) {
  try {
    $z = $num % $zero;
    var_dump($z);
  } catch (DivisionByZeroException $e) {
    var_dump($e->getMessage());
  }
}
<<__EntryPoint>> function main_entry(): void {
main(123, 0);
main(123, 0.0);
main(123, "0");
}
