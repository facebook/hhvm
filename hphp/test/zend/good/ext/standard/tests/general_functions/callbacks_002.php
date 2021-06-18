<?hh
<<__EntryPoint>> function main(): void {
call_user_func(varray['Foo', 'bar']);
call_user_func(varray[NULL, 'bar']);
call_user_func(varray['stdClass', NULL]);
}
