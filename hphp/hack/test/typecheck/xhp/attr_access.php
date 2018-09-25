<?hh
class :foo {
  attribute Map<string, string> bar @required;

  public function doStuff(): void {
    $this->:bar['baz'] = 'herpderp';
  }
}
