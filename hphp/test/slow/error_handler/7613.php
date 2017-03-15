<?php
// `set_error_handler(null)` should clear the error handler and reset it back
// to the default error handler.
set_error_handler(function() { print("Custom handler"); });
set_error_handler(null);
trigger_error("ack");
