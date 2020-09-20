<?hh
class Goo{}
<<__EntryPoint>> function main(): void {
var_dump(call_user_func(new Goo));
}
