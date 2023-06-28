<?hh

<<__EntryPoint>>
function main_unserialize_ex_log_limit() :mixed{
unserialize(str_repeat('x', 5));
unserialize(str_repeat('x', 50000));
}
