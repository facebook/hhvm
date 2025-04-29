<?hh


enum E : int {
  A = 1;
}

type TS = shape('a' => (int,string), 'b' => float);

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

class G<reify T> { }

function cast_enum(mixed $m):E {
  $x = HH\FIXME\UNSAFE_CAST<mixed, E>($m);
  return $x;
}

function cast_vec_int(mixed $m):Traversable<int> {
  $x = HH\FIXME\UNSAFE_CAST<mixed, vec<int>>($m);
  return $x;
}

function cast_tuple(mixed $m):(arraykey,arraykey) {
  $x = HH\FIXME\UNSAFE_CAST<mixed, (int,string)>($m);
  return $x;
}

function cast_shape(mixed $m):TS {
  $x = HH\FIXME\UNSAFE_CAST<mixed, TS>($m);
  return $x;
}

function cast_reified_generic(mixed $m):void {
//  $m as G<int>;
  $mc = HH\FIXME\UNSAFE_CAST<mixed, G<int>>($m);
}

function cast_float(mixed $m):num {
  $x = HH\FIXME\UNSAFE_CAST<mixed, float>(HH\FIXME\UNSAFE_CAST<mixed,num>($m));
  return $x;
}

function cast_function(mixed $m):(function(int):int) {
  $f = HH\FIXME\UNSAFE_CAST<mixed, (function(int):int)>($m);
  return $f;
}

<<__EntryPoint>>
function main():void {
  cast_float(2);
  cast_vec_int(Vector {1,2,3});
  cast_enum(2);
  cast_tuple(tuple('a',23));
  cast_shape(shape('a' => tuple(23, false), 'b' => 2.3));
  cast_function((int $x) ==> $x + 1);
  $c1 = new C1();
  $c2 = new C2();
  $c1->cast_enforceable_typeconst("A");
  $c2->cast_enforceable_typeconst(23);
  $c1->cast_non_enforceable_typeconst(vec["A"]);
  $c2->cast_non_enforceable_typeconst(vec[23]);
  cast_reified_generic(new G<string>());
}
