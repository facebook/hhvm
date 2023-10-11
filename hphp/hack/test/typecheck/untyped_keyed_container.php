<?hh

/* HH_IGNORE_ERROR[4101] */
function foo(KeyedContainer $_): void {
  foo(varray[]);
  foo(varray[]);
  foo(darray[]);
  foo(darray[1 => 2]);
  foo(varray[1, 2, 3]);
  foo(darray['foo' => 'bar']);
}
