<?hh

class MyClass {
    public function __invoke() {
      var_dump('called');
      return $this;
    }
}


<<__EntryPoint>>
function main_invokable() {
(new MyClass())();
}
