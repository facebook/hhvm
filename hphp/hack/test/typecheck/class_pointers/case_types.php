<?hh

class C {}
class D {}
interface I {}

case type X = string | classname<C>;

// Separable once -vEval.ClassPassesClassname=false and similar
case type Y = string | class<C>;
case type Z = class<C> | classname<C>;

// class<T> is a class pointer, never an object, so it is disjoint from
// objects, interfaces and non-string primitives.
case type O = C | class<C>;
case type ObjInterface = I | class<C>;
case type WithInt = int | class<C>;

// Two class pointers with distinct inner types are still indistinguishable at
// runtime (the inner type is not enforced), so they overlap.
case type TwoClasses = class<C> | class<D>;

// A class pointer still shares the string runtime tag during the migration.
case type WithArraykey = arraykey | class<C>;

// class_or_classname<T> is a transparent alias to classname<T> (string), so it
// behaves like classname: disjoint from objects, but overlapping strings/class.
case type CoCObj = C | class_or_classname<C>;
case type CoCClass = class<C> | class_or_classname<C>;

case type S = class<C>;
