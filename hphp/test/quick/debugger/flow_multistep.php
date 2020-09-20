<?hh


function a($a) {
  return b($a) + 1;
}

function b($a) {
  return c($a) + 1;
}

function c($a) {
  return d($a) + 1;
}

function d($a) {
  return $a + 1;
}

class C1 {
private
  $x = 0;

  public function __construct($a) {
    echo "C1 oh hai\n";
    $x = $a;
  }




}

class C2 {
private
  $x = 0;

  public function __construct($a) {
    echo "C2 oh hai\n";
    $x = $a;
  }







}

function main() {
  $c1 = new C1(5);
  $c2 = new C2(5);
  $c2 = new C2(6);
  $d = $c1;
  $c1 = null;
  var_dump($d);
  var_dump(a(42));
}
<<__EntryPoint>>
function entrypoint_flow_multistep(): void {

  main();
}
