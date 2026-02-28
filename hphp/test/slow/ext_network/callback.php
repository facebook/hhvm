<?hh
class Foo { function bar() :mixed{} }

<<__EntryPoint>>
function main_callback() :mixed{
header_register_callback(vec[new Foo, 'bar']);
header_register_callback(vec[new Foo, 'baz']);
}
