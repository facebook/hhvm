<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  let $a: string = "";
  let $a: arraykey = ""; // error: arraykey not subtype of string
}

function g(bool $b): void {

  let $a: string = "";
  if ($b) {
    let $a: arraykey = ""; // error: arraykey not subtype of string
    let $c: int = 1;
  }
  let $c: arraykey = 1; // error: arraykey not subtype of int
}

function h(): void {
  let $d: arraykey = 1;
  let $d: string = 2; // error: 2 not a string
}

function i(): void {
  let $e: arraykey = 1;
  let $e: null = 2; // 2 errors: 2 not null and null not subset of arraykey
}
