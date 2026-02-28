<?hh

<<file: __EnableUnstableFeatures('class_type')>>

function f(class<mixed> $t): class<mixed> {
  return $t;
}
function g(class<mixed> $t): classname<mixed> {
  return $t;
}
function h(classname<mixed> $t): class<mixed> {
  return $t;
}
function j(classname<mixed> $t): classname<mixed> {
  return $t;
}
