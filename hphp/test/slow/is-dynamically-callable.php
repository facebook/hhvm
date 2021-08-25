<?hh

class C { <<__DynamicallyCallable>> function f() {} }

<<__EntryPoint>>
function main() {
  $result = __SystemLib\is_dynamically_callable_inst_method(C::class, 'f');
  print($result ? "true\n" : "false\n");
}
