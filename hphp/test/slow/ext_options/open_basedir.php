<?hh


<<__EntryPoint>>
function main_open_basedir() {
var_dump(ini_set('open_basedir', sys_get_temp_dir()));
var_dump(ini_set('open_basedir', '/home;'.sys_get_temp_dir().';/invalid_root_dir_asdfasdf/dfg;dfg'));
var_dump(ini_get('open_basedir'));

// Can't add since it isn't more specific
var_dump(ini_set('open_basedir', '/home'));
var_dump(ini_get('open_basedir'));

// Can add
var_dump(ini_set('open_basedir', sys_get_temp_dir().'/aasdfasdf'));
var_dump(ini_get('open_basedir'));

// Can't restore
ini_restore('open_basedir');
var_dump(ini_get('open_basedir'));
}
