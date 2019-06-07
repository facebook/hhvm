<?hh


<<__EntryPoint>>
function main_trigger_error_user_constants() {
trigger_error('Herp', E_NOTICE);
trigger_error('Derp', E_USER_NOTICE);
}
