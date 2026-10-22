<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {}

newtype AsArraykey<T> as arraykey = class<T>;
newtype AsString<T> as string = class<T>;
newtype AsClassname<T> as classname<T> = class<T>;
newtype SuperClass super class<C> = string;

newtype ValidClass<T> as class<T> = class<T>;
newtype ValidArraykey as arraykey = int;

case type BadCase as arraykey = class<C>;
case type ValidCase as arraykey = int;
