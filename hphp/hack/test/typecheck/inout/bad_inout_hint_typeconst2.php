<?hh

abstract class C1 {
  abstract const type T as (function(int): void);
}

class C2 extends C1 {
  const type T = (function(inout int): void);
}
