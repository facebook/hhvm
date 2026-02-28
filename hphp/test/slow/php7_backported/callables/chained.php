<?hh

class MyClass {
    public function __invoke() :mixed{
      var_dump('called');
      return $this;
    }
}


<<__EntryPoint>>
function main_chained() :mixed{
(new MyClass())()();
}
