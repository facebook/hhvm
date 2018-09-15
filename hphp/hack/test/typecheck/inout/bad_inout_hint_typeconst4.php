<?hh // strict

abstract class C1 {
  abstract const type T as (function(inout num): void);
}

class C2 extends C1 {
  const type T = (function(inout int): void);
}
