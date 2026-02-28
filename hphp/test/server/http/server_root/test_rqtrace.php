<?hh

<<__EntryPoint>>
function test_rqtrace_entrypoint() :mixed{
var_dump(HH\rqtrace\is_enabled());
var_dump(HH\rqtrace\request_event_stats('REQUEST_QUEUE'));
var_dump(HH\rqtrace\force_enable());
var_dump(HH\rqtrace\is_enabled());
var_dump(HH\rqtrace\request_event_stats('REQUEST_QUEUE'));
}
