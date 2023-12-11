<?hh

abstract final class PushStackStatics {
  public static $index = 0;
  public static $stack = vec[];
}

function push_stack():mixed{

  $val = PushStackStatics::$index++;
  $stack = PushStackStatics::$stack;
  array_push(inout $stack, $val);
  PushStackStatics::$stack = $stack;
}
function pop_stack():mixed{

  if (PushStackStatics::$stack) {
    $stack = PushStackStatics::$stack;
    array_pop(inout $stack);
    PushStackStatics::$stack = $stack;
  }
}
<<__EntryPoint>> function main(): void {
push_stack();
pop_stack();
pop_stack();
pop_stack();
push_stack();
pop_stack();
push_stack();
$info = vec[count(PushStackStatics::$stack), PushStackStatics::$stack[count(PushStackStatics::$stack)-1], PushStackStatics::$stack];
var_dump($info);
}
