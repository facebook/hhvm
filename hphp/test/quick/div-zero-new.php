<?hh

function main($num,$zero) :mixed{
  try {
    $z = HH\Lib\Legacy_FIXME\cast_for_arithmetic($num) / HH\Lib\Legacy_FIXME\cast_for_arithmetic($zero);
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
