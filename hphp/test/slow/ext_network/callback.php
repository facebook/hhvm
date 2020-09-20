<?hh
class Foo { function bar() {} }

<<__EntryPoint>>
function main_callback() {
header_register_callback(varray[new Foo, 'bar']);
header_register_callback(varray[new Foo, 'baz']);
}
