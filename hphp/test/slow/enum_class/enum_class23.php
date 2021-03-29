<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IBox {
};

class Box<T> implements IBox {
    public function __construct(public T $data)[] {
    }
};

enum class Foo: \HH\SwitchableClass<IBox> {
    HH\SwitchableClass<Box<string>> foo =
        new HH\SwitchableClass(new Box("foo"));
};

enum class FooBar: \HH\SwitchableClass<IBox> extends Foo {
    HH\SwitchableClass<Box<string>> bar =
        new HH\SwitchableClass(new Box("bar"));
};

function getFoo_foo(): \HH\MemberOf<Foo, \HH\SwitchableClass<Box<string>>> {
    return Foo::foo;
};

function getFooBar_foo(): \HH\MemberOf<Foo, \HH\SwitchableClass<Box<string>>> {
    return FooBar::foo;
};

<<__EntryPoint>>
function main(): void {
    try {
        if (getFoo_foo() == getFooBar_foo()) {
            echo "same\n";
        }
        else {
            echo "not same\n";
        }
        if (getFoo_foo() === getFooBar_foo()) {
            echo "identical\n";
        }
        else {
            echo "not identical\n";
        }
    } catch (Exception $e) {
        echo 'Exception: '.$e->getMessage()."\n";
    }
}
