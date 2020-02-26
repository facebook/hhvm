<?hh

class Test {

  public function __call($method, $args) {
    var_dump($args);
  }

  public function normal($args) {
    var_dump($args);
  }

}
<<__EntryPoint>> function main(): void {
$test = new Test();
call_user_func_array(varray[$test, 'magic'], darray['bur' => 'bar']);
call_user_func_array(varray[$test, 'normal'], darray['badum' => 'tss']);
$test->hi('hello world!');
}
