<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {}

interface Bounds {
  abstract const type AsArraykey as arraykey;
  abstract const type AsString as string;
  abstract const type AsClassname as classname<C>;
  abstract const type SuperClass super class<C>;
  abstract const type ValidClass as class<C>;
  abstract const type ValidArraykey as arraykey;
}

class Implementation implements Bounds {
  const type AsArraykey = class<C>;
  const type AsString = class<C>;
  const type AsClassname = class<C>;
  const type SuperClass = string;
  const type ValidClass = class<C>;
  const type ValidArraykey = int;
}
