<?hh

function foo($a) :mixed{
  $result = dict[];


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
    return dict['success' => true];//$result;
  }

  $result['success'] = false;

  return $result;
}

function throwError(Exception $ex) :mixed{
  throw $ex;
}


<<__EntryPoint>>
function main_subtypeof() :mixed{
var_dump(foo(42));
}
