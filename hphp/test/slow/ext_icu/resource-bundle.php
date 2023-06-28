<?hh

<<__EntryPoint>>
function main_resource_bundle() :mixed{
var_dump(count(\ResourceBundle::getLocales(''))>0);
}
