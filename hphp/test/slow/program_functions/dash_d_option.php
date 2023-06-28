<?hh


<<__EntryPoint>>
function main_dash_d_option() :mixed{
var_dump(ini_get("hhvm.log.level"));
var_dump(ini_get("this.should.not.work"));
}
