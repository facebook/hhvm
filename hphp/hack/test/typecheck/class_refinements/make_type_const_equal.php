<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type TData;
}

class Inv<T> {}

function get<T as Box, TData>(Inv<T> $inv): TData where TData = T::TData {
  while (true) {}
}

function makeiib(): Inv<Box with { type TData = int }> {
  while (true) {}
}

function test(): void {
  // the call to get() will generate two type variables:
  // #1 for T and #2 for TData. Subsequently, the subtype
  // engine sees #1 <: Box with { type TData = int },
  // and decomposes into #1 <: Thas_type_member('TData',int)
  // and #1 <: Box. The former causes a call
  // Typing_subtype_tconst.make_type_const_equal; which
  // we intend to test here.
  //
  // phew.
  get(makeiib());
}
