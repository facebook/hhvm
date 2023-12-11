<?hh

// taking default args
class C3 {
  public function __invoke($a0, $a1 = vec[], $a2 = false) :mixed{
    var_dump($a0, $a1, $a2);
  }
}

<<__EntryPoint>>
function main_766() :mixed{
$c = new C3;
$c(0);
$c(0, vec[1]);
$c(0, vec[1], true);
call_user_func($c, 0);
call_user_func($c, 0, vec[1]);
call_user_func($c, 0, vec[1], true);
}
