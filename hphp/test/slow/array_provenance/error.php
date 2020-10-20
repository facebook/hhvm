<?hh

function make_me_an_exception() {
  try {
    $x = varray[17, 34];
    return $x[2];
  } catch (Exception $e) {
    return $e;
  }
}

function read_me_an_exception_0($e) {
  return HH\get_provenance($e->getTrace());
}

function read_me_an_exception_1($e) {
  return HH\get_provenance($e->getTrace());
}

<<__EntryPoint>>
function main() {
  $e = make_me_an_exception();
  print(read_me_an_exception_0($e)."\n");
  print(read_me_an_exception_1($e)."\n");
}
