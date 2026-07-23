<?hh

<<file: __EnableUnstableFeatures('class_type', 'union_intersection_type_hints')>>

class Parent_ {}
class Child_ extends Parent_ {}

function accept_class_or_classname<T>(class_or_classname<T> $x): void {}

// class<C> <: class_or_classname<C>
function test_class_sub(class<Child_> $c): void {
  accept_class_or_classname<Child_>($c);
}

// classname<C> <: class_or_classname<C>
function test_classname_sub(classname<Child_> $cn): void {
  accept_class_or_classname<Child_>($cn);
}

// class<ChildClass> <: class_or_classname<ParentClass> (covariance)
function test_class_covariant(class<Child_> $c): void {
  accept_class_or_classname<Parent_>($c);
}

// classname<ChildClass> <: class_or_classname<ParentClass> (covariance)
function test_classname_covariant(classname<Child_> $cn): void {
  accept_class_or_classname<Parent_>($cn);
}

class A {}
class B {}

// class<A> | class_or_classname<B> simplifies to class_or_classname<A | B>
function test_union_simplification(
  bool $cond,
  class<A> $c,
  class_or_classname<B> $cocn,
): void {
  $x = $cond ? $c : $cocn;
  accept_class_or_classname<(A | B)>($x);
}
