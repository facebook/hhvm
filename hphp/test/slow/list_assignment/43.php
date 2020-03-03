<?hh

class SetTest {
  private $_vals = darray['one' => 1, 'two' => 2, 'three' => 3];
  public function __set($name, $value) {
    $this->_vals[$name] = $value;
  }
}

<<__EntryPoint>>
function main_43() {
  $q = varray['eins', 'zwei', varray['drei']];
  var_dump($q);
  $x = new SetTest;
  $qq = list($x->one, $x->two, list($x->three)) = 1;
  var_dump($x);
  $qq = list($x->one, $x->two, list($x->three)) = $q;
  var_dump($x);
  var_dump($qq);
}
