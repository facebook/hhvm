<?hh

class X {}
enum Y : int {}

type MyTypeNoAttrs = X;

<<Attr1(1337)>>
type MyType = X;

<<Attr2, Attr3>>
newtype MyNewType = Y;

<<Attr2, Attr3>>
type MyOtherType = MyTypeNoAttrs;

<<Attr2, Attr3>>
type MyOtherOtherType = MyNewType;
<<__EntryPoint>> function main(): void {
var_dump((new ReflectionTypeAlias('MyTypeNoAttrs'))->getAttributes());
var_dump((new ReflectionTypeAlias('MyType'))->getAttributes());
var_dump((new ReflectionTypeAlias('MyNewType'))->getAttributes());
var_dump((new ReflectionTypeAlias('MyOtherType'))->getAttributes());
var_dump((new ReflectionTypeAlias('MyOtherOtherType'))->getAttributes());
}
