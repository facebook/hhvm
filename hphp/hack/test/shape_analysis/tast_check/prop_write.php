<?hh

class C {
  public dict<string, mixed> $instance_prop = dict[];
  public static dict<string, mixed> $class_prop = dict[];
}

function instance_write(C $c): void {
  $d = dict[];
  $c->instance_prop = $d;
}

function class_write(): void {
  $d = dict[];
  C::$class_prop = $d;
}
