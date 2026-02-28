<?hh

enum E : int {
  A = 1;
}

class G<reify T> { }

abstract class A {
  // We can enforce this type const
  <<__Enforceable>> abstract const type TC;
  // But not this one: we should replace type const by _
  // when checking UNSAFE_CAST<_,this::TC2>
  abstract const type TC2;
  public function cast_enforceable_typeconst(mixed $m):void {
    $x = HH\FIXME\UNSAFE_CAST<mixed, this::TC>($m);
  }
  public function cast_non_enforceable_typeconst(mixed $m):void {
    $x = HH\FIXME\UNSAFE_CAST<mixed, this::TC2>($m);
  }
}
class C1 extends A {
  const type TC = int;
  const type TC2 = vec<int>;
}
class C2 extends A {
  const type TC = string;
  const type TC2 = vec<string>;
}

function cast_enum(mixed $m):E {
  $m as E;
  $x = HH\FIXME\UNSAFE_CAST<mixed, E>($m);
  return $x;
}

// Should just ignore that this is a generic
function cast_generic<T>(mixed $m):T {
  $x = HH\FIXME\UNSAFE_CAST<mixed, T>($m);
  return $x;
}

function cast_function_generic<Tout>(mixed $m):mixed {
  return HH\FIXME\UNSAFE_CAST<mixed, (function()[defaults]: Tout)>($m);
}

function cast_vec_int(mixed $m):vec<int> {
  $x = HH\FIXME\UNSAFE_CAST<mixed, vec<int>>($m);
  return $x;
}

function cast_tuple(mixed $m):(arraykey,arraykey) {
  $x = HH\FIXME\UNSAFE_CAST<mixed, (int,string)>($m);
  return $x;
}

function cast_float(mixed $m):float {
  $x = HH\FIXME\UNSAFE_CAST<mixed, float>($m);
  return $x;
}

function cast_keyed_traversable(mixed $m):KeyedTraversable<nothing,nothing> {
  $x = HH\FIXME\UNSAFE_CAST<mixed, KeyedTraversable<nothing, nothing>>($m);
  return $x;
}

class C {
  const type TC = arraykey;
}
function cast_type_structure(mixed $m):TypeStructure<arraykey> {
  $ts = type_structure(C::class, 'TC');
  $tsc = HH\FIXME\UNSAFE_CAST<mixed, TypeStructure<arraykey>>($ts);
  return $tsc;
}

function cast_reified_generic(mixed $m):void {
//  $m as G<int>;
  $mc = HH\FIXME\UNSAFE_CAST<mixed, G<int>>($m);
}

<<__EntryPoint>>
function main():void {
  var_dump(cast_float(2.3));
  var_dump(cast_vec_int(vec[2,3]));
  var_dump(cast_enum(E::A));
  var_dump(cast_tuple(tuple(2, "A")));
  cast_generic<int>(3);
  cast_function_generic<int>(function() {
    return 3;
  });
  cast_reified_generic(new G<int>());
  var_dump(cast_keyed_traversable(Map { 2 => 'a' }));
  $c1 = new C1();
  $c2 = new C2();
  $c1->cast_enforceable_typeconst(23);
  $c2->cast_enforceable_typeconst("A");
  $c1->cast_non_enforceable_typeconst(vec[23]);
  $c2->cast_non_enforceable_typeconst(vec["A"]);
}
