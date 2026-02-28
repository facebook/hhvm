<?hh

class Klass {
  public function foo(): void {
    $ignore1 = 1;
    $param1 = 1;
    $param2 = 1;
    $param3_and_ret1 = 1;
    /*range-start*/
    $param3_and_ret1 = $param3_and_ret1 + $param1;
    $local = $param3_and_ret1 + $param2;
    $param3_and_ret1 = $local;
    $ret2 = 1;
    /*range-end*/
    $ignore2 = 0;
    $ignore3 = $param3_and_ret1;
    $ignore3 = $ret2;
  }
}
