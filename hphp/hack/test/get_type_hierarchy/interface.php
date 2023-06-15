<?hh

interface IFoo {
  const const_from_IFoo = 1;

  function method_from_IFoo(): void;
}

interface IBar {
  const const_from_IBar = 2;

  function method_from_IBar(): void;
}

interface IQux extends IFoo, IBar {
  //         ^ type-hierarchy-at-caret
  const const_from_IQux = 2;

  function method_from_IQux(): void;
}
