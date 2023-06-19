<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

function f(): void {
  let $v: vec<arraykey> = vec[];
  $v[] = null;
}

function g(): void {
  let $v: Vector<arraykey> = Vector {};
  $v[] = null;
}

function h(): void {
  let $v: vec<string> = vec<arraykey>[];
}

function i(): void {
  let $v: Vector<string> = Vector<arraykey> {};
}

function j(): void {
  let $v: Vector<arraykey> = Vector<string> {};
}
