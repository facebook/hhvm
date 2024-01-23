<?hh

function neg_zero() :mixed{
  $zero = -1.0;
  while ($zero != 0.0) {
    $zero /= 2.0;
  }
  return $zero;
}

function main($f) :mixed{
  var_dump(sqrt(0.0));
  var_dump(sqrt(neg_zero()));
  var_dump(sqrt(5.0));
  var_dump(sqrt(2.5));
  var_dump(sqrt(-3.0));
  var_dump(sqrt($f));
  try {
    var_dump(sqrt(true));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(sqrt(false));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(sqrt(null));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(sqrt("15"));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(sqrt("hello"));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(sqrt(new stdClass));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(sqrt(vec[]));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(sqrt(vec[2,3,4]));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });
  main(2.241987);
}
