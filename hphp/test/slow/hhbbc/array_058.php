<?hh

class MyClass {
  private function getKeyPrefix() {
    return 'foo:';
  }

  public function makeArray() {
    $data = darray[];
    $p = $this->getKeyPrefix();

    $data[$p.'a'] = 2;
    $data[$p.'b'] = 3;
    $data[$p.'c'] = 4;
    $data[$p.'d'] = 5;

    return $data;
  }
}


<<__EntryPoint>>
function main_array_058() {
var_dump((new MyClass)->makeArray());
}
