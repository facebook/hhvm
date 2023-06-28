<?hh
class Foo { function bar() :mixed{} }

<<__EntryPoint>>
function main_callback() :mixed{
header_register_callback(varray[new Foo, 'bar']);
header_register_callback(varray[new Foo, 'baz']);
}
