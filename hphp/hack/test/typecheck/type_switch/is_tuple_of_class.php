<?hh

abstract class MyAbstract {
  abstract const type TOutput;

  abstract public function getOutput(): this::TOutput;
}

function my_fun2(MyAbstract $x): (int, dict<arraykey, mixed>) {
  $output = tuple(0, $x->getOutput());
  if ($output is (int, dict<_, _>)) {
    return $output;
  }
  throw new Exception();
}
