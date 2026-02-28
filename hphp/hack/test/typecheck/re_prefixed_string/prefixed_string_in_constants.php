<?hh

class MyClass {
  const HH\Lib\Regex\Pattern<shape(...)> PATTERN = re'/[a-z]+/';
  const HH\Lib\Regex\Pattern<shape('x' => string, ...)> NAMED =
    re'/(?P<x>[a-z]+)/';
}
