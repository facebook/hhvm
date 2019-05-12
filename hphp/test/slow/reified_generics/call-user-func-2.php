<?hh

function f<reify T>() { echo "done\n"; }
<<__EntryPoint>> function main(): void {
call_user_func("f");
}
