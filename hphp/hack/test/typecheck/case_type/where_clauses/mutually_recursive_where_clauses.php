<?hh

case type A<T> = int where T as B<int> | bool;
case type B<T> = string where T as A<string> | null;

function f(): void {
  // unconditional cases
  hh_expect<A<nothing>>(false);
  hh_expect<B<nothing>>(null);

  // obvious cases
  hh_expect<A<B<int>>>(3);
  hh_expect<B<A<string>>>("");

  // would be infinite case
  hh_expect<A<string>>(0); // error
  // int < A<string>
  // iff int < int &&& string < B<int>
  // iff int < int &&& string < string &&& int < A<string>
  //                                       ^^^^^^^^^^^^^^^ recursion

  // unconditional even in case of poorly defined generic
  hh_expect<A<string>>(true);
}
