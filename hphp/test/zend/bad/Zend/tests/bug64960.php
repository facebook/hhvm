<?php
// this makes ob_end_clean raise an error
ob_end_flush();

class ExceptionHandler {
	public function __invoke (Exception $e)
	{
		// this triggers the custom error handler
		ob_end_clean();
	}
}

// this must be a class, closure does not trigger segfault
set_exception_handler(new ExceptionHandler());

// exception must be throwed from error handler.
set_error_handler(function()
{
	$e = new Exception;
	$e->_trace = debug_backtrace();
	
	throw $e;
});

// trigger error handler
$a['waa'];
?>