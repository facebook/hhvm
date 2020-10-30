#! this is ok, hashbang is supported
<?hh
# not-a-hashbang

<<__EntryPoint>>
function main_hashbang_regression() : void {
var_dump(true);
}
