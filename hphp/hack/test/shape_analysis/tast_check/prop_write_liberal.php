<?hh

class C {
  public mixed $instance_prop = dict[];
  public static mixed $class_prop = dict[];
}

function instance_write(C $c): void {
  $d = dict[];
  // Do not invalidate the dict above because flows into mixed
  $c->instance_prop = $d;
}

function class_write(): void {
  $d = dict[];
  // Do not invalidate the dict above because flows into mixed
  C::$class_prop = $d;
}
