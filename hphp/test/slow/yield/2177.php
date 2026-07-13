<?hh

class IntRef {
  public function __construct(public int $val)[] {}
}

function makeClosureCont() :mixed{
  $ref = new IntRef(0);
  return function () use ($ref) {
    $t = $ref->val; $ref->val++; yield $t;
    $t = $ref->val; $ref->val++; yield $t;
  }
;
}

abstract final class GenStatics {
  public static $x = 0;
}
function gen() :AsyncGenerator<mixed,mixed,void>{
  $t = GenStatics::$x; GenStatics::$x++; yield $t;
  $t = GenStatics::$x; GenStatics::$x++; yield $t;
}

<<__EntryPoint>>
function main_2177() :mixed{
$cc = makeClosureCont();
foreach ($cc() as $v) {
 var_dump($v);
 }
$cc1 = makeClosureCont();
foreach ($cc1() as $v) {
 var_dump($v);
 }
foreach (gen() as $v) {
 var_dump($v);
 }
foreach (gen() as $v) {
 var_dump($v);
 }
}
