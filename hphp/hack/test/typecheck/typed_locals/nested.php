<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function get_bool(): bool {
  return true;
}

function f(): void {
  let $a: arraykey = "";
  while (get_bool()) {
    if (get_bool()) {
      continue;
    } else {
      let $a: int = 1;
    }
  }
  $a = "";
}

interface I1 {}

interface I2 {}

interface I3 {}

interface I4 {}

class C1 implements I1, I2, I3, I4 {}

class C2 implements I1, I2, I3, I4 {}

class C3 implements I1, I2, I3, I4 {}

class D implements I1, I3, I4 {}

function g(): void {
  if (get_bool()) {
    if (get_bool()) {
      let $x : I1 = new C1();
    } else {
      let $x : I2 = new C2();
    }
  } else {
    if (get_bool()) {
      let $x : I3 = new C3();
    } else {
      let $x : I4 = new D();
    }
  }
}
