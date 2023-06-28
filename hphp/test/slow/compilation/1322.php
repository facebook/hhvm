<?hh

class X {
  public function foo($offset) :mixed{
    if (isset($this->__array[$offset])) {
      return $this->initializeOffset($offset);
    }
 else {
      return null;
    }
    return $this->__array[$offset];
  }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
