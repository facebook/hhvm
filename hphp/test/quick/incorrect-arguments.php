<?hh
<<__EntryPoint>>
function main(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });
  try {
    is_null();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump(is_null(null));
  try {
    is_null(null, null);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    is_null(null, null, null);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    strpos();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    strpos(null);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(strpos(null, null));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    strpos(null, null, null);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    strpos(null, null, null, null);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
