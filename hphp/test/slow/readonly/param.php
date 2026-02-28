<?hh

class P {
  public function __construct(public int $i) { }
}


function return_mutable_error(readonly P $x): P {
  // error
  return $x;
}

function return_readonly(readonly P $x) : readonly P {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  return_readonly();
  return_mutable_error();
}
