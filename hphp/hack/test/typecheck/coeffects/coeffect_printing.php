<?hh

function a()[]: void {
  $x = a<>;
  hh_show($x);
}

function b()[write_props]: void {
  $x = b<>;
  hh_show($x);
}

function c()[zoned]: void {
  $x = c<>;
  hh_show($x);
}

function d()[defaults]: void {
  $x = d<>;
  hh_show($x);
}

function e(): void {
  $x = e<>;
  hh_show($x);
}
