<?hh

class foo {
  const foobar=1;
  public $pp = darray['t'=>null];

  function bar() {
    echo $this->t ='f';
  }
  function __get($prop) {
    return $this->pp[$prop];
  }
  function __set($prop, $val) {
    echo "__set";
    $this->pp[$prop] = '';
  }
}

<<__EntryPoint>> function main(): void {
$f = 'c="foo"';
$f = new foo;
$f->bar();
}
