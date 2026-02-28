<?hh

class MyClass {
  private function getKeyPrefix() :mixed{
    return 'foo:';
  }

  public function makeArray() :mixed{
    $data = dict[];
    $p = $this->getKeyPrefix();

    $data[$p.'a'] = 2;
    $data[$p.'b'] = 3;
    $data[$p.'c'] = 4;
    $data[$p.'d'] = 5;

    return $data;
  }
}


<<__EntryPoint>>
function main_array_058() :mixed{
var_dump((new MyClass)->makeArray());
}
