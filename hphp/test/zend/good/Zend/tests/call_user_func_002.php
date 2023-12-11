<?hh

<<__EntryPoint>> function main(): void {
call_user_func(vec['foo', 'bar']);
call_user_func(vec['', 'bar']);
call_user_func(vec[$foo, 'bar']);
call_user_func(vec[$foo, '']);
}
