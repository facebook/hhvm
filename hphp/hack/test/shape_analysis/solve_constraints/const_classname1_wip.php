<?hh

class Foo {
    const dict<string, mixed> DICT = dict[]; // expect key ?'b'
}

function f(classname<Foo> $the_class_name): void {
  $the_class_name::DICT['b']; // should be Shapes::idx
}
