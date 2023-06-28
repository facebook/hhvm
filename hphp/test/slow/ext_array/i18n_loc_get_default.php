<?hh

<<__EntryPoint>>
function main_i18n_loc_get_default() :mixed{
var_dump(i18n_loc_set_default("en_UK"));
var_dump(i18n_loc_get_default());
var_dump(i18n_loc_set_default("en_US"));
var_dump(i18n_loc_get_default());
}
