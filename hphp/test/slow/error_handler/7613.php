<?hh

// `set_error_handler(null)` should clear the error handler and reset it back
// to the default error handler.
<<__EntryPoint>>
function main_7613() :mixed{
set_error_handler(function() { print("Custom handler"); });
set_error_handler(null);
trigger_error("ack");
}
