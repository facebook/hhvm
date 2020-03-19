<?hh

class MyC {
}

function call_constructor(): void {
  $args = vec[1, 2, 3];
  new MyC(...$args);
}
