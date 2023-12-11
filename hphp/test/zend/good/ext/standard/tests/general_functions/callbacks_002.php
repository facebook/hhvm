<?hh
<<__EntryPoint>> function main(): void {
call_user_func(vec['Foo', 'bar']);
call_user_func(vec[NULL, 'bar']);
call_user_func(vec['stdClass', NULL]);
}
