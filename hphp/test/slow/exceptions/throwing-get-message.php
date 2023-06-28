<?hh

class E extends Exception {
  public function getMessage() :mixed{
    throw new Exception("clowntown");
  }
}

<<__EntryPoint>>
function main() :mixed{
  throw new E();
}
