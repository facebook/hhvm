<?hh

<<__EntryPoint>>
function main_resource_bundle() {
var_dump(count(\ResourceBundle::getLocales(''))>0);
}
