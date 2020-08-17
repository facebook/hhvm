<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

/* If a class takes even one reified parameter, all hints where the class is
 * present must be parameterized */

class Erased<T> {}
class Reified<reify Tr> {}

function Erased_parametrizedCheck(): void {
  new Erased<Erased<_>>();

  new Erased<Reified>(); // bad
  new Erased<Reified<int>>();
}

function Reified_parametrizedCheck(): void {
  new Reified<Erased>();  // bad

  new Reified<Reified>(); // bad
  new Reified<Reified<int>>();
}

// just some sanity checks
function deepChecks(): void {
  new Erased<Erased<Erased>>(); // bad
  new Erased<Erased<Erased<Reified>>>(); // bad
  new Erased<Erased<Erased<Reified<int>>>>();

  new Reified<Erased<Reified<int>>>();
  new Reified<Erased<Erased<Reified>>>(); // bad
  new Reified<Erased<Erased<Reified<int>>>>();
}
