<?hh

function f(readonly classname<mixed> $s): void {
  $_ = HH\classname_to_class($s);
}

function g(readonly classname<mixed> $c): void {
  $_ = HH\class_to_classname($c);
}
