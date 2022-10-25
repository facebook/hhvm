<?hh

class C {
  public dict<string, mixed> $instance_prop = dict[];
  public static dict<string, mixed> $class_prop = dict[];
}

function instance_write(C $c): void {
  vec[dict[], $c->instance_prop];
}

function class_write(): void {
  vec[dict[], C::$class_prop];
}
