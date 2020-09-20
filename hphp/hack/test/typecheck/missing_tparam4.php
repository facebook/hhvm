<?hh // strict

newtype Foo<T> as string = string;

enum Bar: Foo {
}
