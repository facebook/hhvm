#! this is ok, hashbang is supported
<?hh
# not_a_hashbang

<<__EntryPoint>>
function main_hashbang_regression() : void {
var_dump(true);
}
