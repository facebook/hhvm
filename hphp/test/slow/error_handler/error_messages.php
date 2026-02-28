<?hh


<<__EntryPoint>>
function main_error_messages() :mixed{
trigger_error("E_USER_DEPRECATED", E_USER_DEPRECATED);
trigger_error("E_USER_ERROR", E_USER_ERROR);
}
