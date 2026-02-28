<?hh

class C {}

case type X = string | classname<C>;

// Separable once -vEval.ClassPassesClassname=false and similar
case type Y = string | class<C>;
case type Z = class<C> | classname<C>;

// Currently separable w/ HH\is_class, but won't bother with these until we
// admit is/as
case type O = C | class<C>;

case type S = class<C>;
