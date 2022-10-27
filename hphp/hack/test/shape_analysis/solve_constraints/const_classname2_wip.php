<?hh

class Foo {
    const dict<string, mixed> DICT = dict[]; // expect key ?'b'
}

class Bar extends Foo {
    const dict<string, mixed> DICT = dict[]; // expect key ?'b'
}

function f(classname<Foo> $the_class_name): void {
  $the_class_name::DICT['b'];
}

function main(classname<Bar> $the_class_name): void {
  f(Bar::class);
}
