<?hh

class IntRef {
  public function __construct(public int $val)[] {}
}

function makeClosureCont() :mixed{
  $ref = new IntRef(0);
  return function () use ($ref) {
    yield $ref->val++;
    yield $ref->val++;
  }
;
}

abstract final class GenStatics {
  public static $x = 0;
}
function gen() :AsyncGenerator<mixed,mixed,void>{
  yield GenStatics::$x++;
  yield GenStatics::$x++;
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
