<?hh

class P {
  public function __construct(public int $i) { }
}


function return_mutable_error(): P {
  // error
  return readonly new P(0);
}

function return_readonly() : readonly P {
    return readonly new P(0);
}

<<__EntryPoint>>
function main() :mixed{
  return_readonly();
  return_mutable_error();
}
