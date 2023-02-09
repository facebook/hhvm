<?hh

class E extends Exception {
  public function getMessage() {
    throw new Exception("clowntown");
  }
}

<<__EntryPoint>>
function main() {
  throw new E();
}
