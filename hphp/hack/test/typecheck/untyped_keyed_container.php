<?hh // strict

/* HH_IGNORE_ERROR[4101] */
function foo(KeyedContainer $_): void {
  foo([]);
  foo(varray[]);
  foo(darray[]);
  foo([1 => 2]);
  foo(varray[1, 2, 3]);
  foo(darray['foo' => 'bar']);
}
