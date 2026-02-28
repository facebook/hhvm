<?hh

<<__NEVER_INLINE>>
function rfunc1<reify Ta, reify Tb>() :mixed{ echo __FUNCTION__."\n"; }
<<__NEVER_INLINE>>
function rfunc2<reify Ta, Tb>() :mixed{ echo __FUNCTION__."\n"; }
<<__NEVER_INLINE>>
function rfunc3<Ta, reify Tb>() :mixed{ echo __FUNCTION__."\n"; }
<<__NEVER_INLINE>>
function rfunc4<Ta, Tb>() :mixed{ echo __FUNCTION__."\n"; }
<<__NEVER_INLINE>>
function rfunc5<Ta, reify Tb, Tc>() :mixed{ echo __FUNCTION__."\n"; }
<<__NEVER_INLINE>>
function rfunc6<reify Ta, Tb, reify Tc>() :mixed{ echo __FUNCTION__."\n"; }

class T {
  <<__NEVER_INLINE>>
  function rmeth1<reify Ta, reify Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  function rmeth2<reify Ta, Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  function rmeth3<Ta, reify Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  function rmeth4<Ta, Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  function rmeth5<Ta, reify Tb, Tc>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  function rmeth6<reify Ta, Tb, reify Tc>() :mixed{ echo __METHOD__."\n"; }

  <<__NEVER_INLINE>>
  static function rsmeth1<reify Ta, reify Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  static function rsmeth2<reify Ta, Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  static function rsmeth3<Ta, reify Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  static function rsmeth4<Ta, Tb>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  static function rsmeth5<Ta, reify Tb, Tc>() :mixed{ echo __METHOD__."\n"; }
  <<__NEVER_INLINE>>
  static function rsmeth6<reify Ta, Tb, reify Tc>() :mixed{ echo __METHOD__."\n"; }
}

class A {}
interface I {}
trait Q {}
enum E : int {}

type X = int;
newtype Y = A;
class TC { const type T = Y; }

class :Z {}

<<__EntryPoint>>
function main() :mixed{
  fb_setprofile(($mode, $fn, $frame) ==> var_dump($frame['reified_classes'] ?? null), SETPROFILE_FLAGS_ENTERS);

  rfunc1<A, E>();
  rfunc2<I, Q>();
  rfunc3<E, Q>();
  rfunc4<A, Q>();
  rfunc5<A, TC::T, E>();
  rfunc6<X, A, Y>();

  T::rsmeth1<A, E>();
  T::rsmeth2<I, Q>();
  T::rsmeth3<E, Q>();
  T::rsmeth4<A, Q>();
  T::rsmeth5<A, TC::T, E>();
  T::rsmeth6<X, A, Y>();

  (new T)->rmeth1<A, E>();
  (new T)->rmeth2<I, Q>();
  (new T)->rmeth3<E, Q>();
  (new T)->rmeth4<A, Q>();
  (new T)->rmeth5<A, TC::T, E>();
  (new T)->rmeth6<X, A, Y>();


  rfunc1<:Z, :Z>();
  T::rsmeth1<:Z, :Z>();
  (new T)->rmeth1<:Z, :Z>();
}
