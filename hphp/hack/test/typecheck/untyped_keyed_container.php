<?hh

/* HH_FIXME[4101] */
function foo(KeyedContainer $_): void {
  foo(vec[]);
  foo(vec[]);
  foo(dict[]);
  foo(dict[1 => 2]);
  foo(vec[1, 2, 3]);
  foo(dict['foo' => 'bar']);
}
