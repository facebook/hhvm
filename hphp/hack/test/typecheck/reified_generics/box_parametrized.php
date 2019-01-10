<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

/* If a class takes even one reified parameter, all hints where the class is
 * present must be parameterized */

class Erased<T> {}
class Reified<reify Tr> {}

function Erased_parametrizedCheck(): void {
  new Erased<Erased>();

  new Erased<Reified>(); // bad
  new Erased<Reified<int>>();
}

function Reified_parametrizedCheck(): void {
  new Reified<reify Erased>();

  new Reified<reify Reified>(); // bad
  new Reified<reify Reified<int>>();
}

// just some sanity checks
function deepChecks(): void {
  new Erased<Erased<Erased>>();
  new Erased<Erased<Erased<Reified>>>(); // bad
  new Erased<Erased<Erased<Reified<int>>>>();

  new Reified<reify Erased<Reified<int>>>();
  new Reified<reify Erased<Erased<Reified>>>(); // bad
  new Reified<reify Erased<Erased<Reified<int>>>>();
}
