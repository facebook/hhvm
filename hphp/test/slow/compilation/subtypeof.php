<?hh

function foo($a) {
  $result = darray[];


  try {
    if ($a & 2) {
      $result['success'] = true;
    } else {
      $result['success'] = false;
      return $result;
    }
  } catch (Exception $ex) {
    throwError($ex);
  }

  if ($a & 4) {
    return darray['success' => true];//$result;
  }

  $result['success'] = false;

  return $result;
}

function throwError(Exception $ex) {
  throw $ex;
}


<<__EntryPoint>>
function main_subtypeof() {
var_dump(foo(42));
}
