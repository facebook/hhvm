<?hh

class X { static function y() :mixed{} }
<<__DynamicallyCallable>> function t() :mixed{ var_dump(__FUNCTION__); }

<<__EntryPoint>>
function main() :mixed{
  (new ReflectionFunction('t'))->invoke();
  var_dump(HH\classname_to_class(HH\classname_to_class('X')));
}
