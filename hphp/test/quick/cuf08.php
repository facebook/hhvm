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
call_user_func_array(array($test, 'magic'), array('bur' => 'bar'));
call_user_func_array(array($test, 'normal'), array('badum' => 'tss'));
$test->hi('hello world!');
}
