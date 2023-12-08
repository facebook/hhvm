<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I<+T> { }
interface J extends I<arraykey> { }
class Base<T> implements I<T> { }
// Should this be allowed?
// We were thinking that implements should override inherited stuff
// But perhaps only when there is a direct instantiation in the declaration
class Derived extends Base<string> implements J { }

class Another extends Base<string> { }
// On the other hand, what about this?
class Derived2 extends Another implements J { }

// Workaround is to re-implement e.g.
class Derived3 extends Base<string> implements I<string>, J { }

function expect(I<string> $_):void { }

function test1(Derived $d):void {
  expect($d);
  }
