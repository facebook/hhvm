<?hh // partial

<<__Native>>
function __SystemLib_filter_input_get_var(int $variable_name) : array;

/**
 * You almost never want this. It acts as though the request's globals
 * should be snapshotted now. Useful for unit tests that want to muck with
 * globals, and then lock them in.
 */
<<__Native>>
function _filter_snapshot_globals() : void;
