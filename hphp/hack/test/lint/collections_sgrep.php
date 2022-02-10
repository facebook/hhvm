<?hh

function test() {
  new Vector(null);
  new Map(null);
  new Set(null);
  new ImmVector(null);
  new ImmMap(null);
  new ImmSet(null);
  new Vector(varray[]);
  new Map(varray[]);
  new Set(varray[]);
  new ImmVector(varray[]);
  new ImmMap(varray[]);
  new ImmSet(varray[]);
  new Set(array_keys(X));
  new Vector(array_keys(X));
  new Set(array_values(X));
  new Vector(array_values(X));
  new Map();
  new Set();
  new Vector();
  new ImmMap();
  new ImmSet();
  new ImmVector();
}
