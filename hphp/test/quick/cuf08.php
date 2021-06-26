<?hh

class Test {
  <<__DynamicallyCallable>>
  public function normal($args) {
    var_dump($args);
  }

}
<<__EntryPoint>> function main(): void {
$test = new Test();
call_user_func_array(varray[$test, 'normal'], darray['badum' => 'tss']);
}
