<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {}

abstract class Defaults {
  abstract const type AsArraykey as arraykey = class<C>;
  abstract const type AsString as string = class<C>;
  abstract const type AsClassname as classname<C> = class<C>;
  abstract const type SuperClass super class<C> = string;

  abstract const type ValidClass as class<C> = class<C>;
  abstract const type ValidArraykey as arraykey = int;
}
