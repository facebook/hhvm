<?hh

<<__DynamicallyCallable>> function backtrace_print($opt = null)
:mixed{
	if(is_null($opt)) {
		print_r(debug_backtrace());
	} else {
		print_r(debug_backtrace($opt));
	}
	__hhvm_intrinsics\launder_value($opt);
}

<<__DynamicallyCallable>> function doit($a, $b, $how)
:mixed{
	echo "==default\n";
	HH\dynamic_fun($how)();
	echo "==1\n";
	HH\dynamic_fun($how)(1);
	echo "==0\n";
	HH\dynamic_fun($how)(0);
	echo "==DEBUG_BACKTRACE_PROVIDE_OBJECT\n";
	HH\dynamic_fun($how)(DEBUG_BACKTRACE_PROVIDE_OBJECT);
	echo "==DEBUG_BACKTRACE_IGNORE_ARGS\n";
	HH\dynamic_fun($how)(DEBUG_BACKTRACE_IGNORE_ARGS);
	echo "==both\n";
	HH\dynamic_fun($how)(DEBUG_BACKTRACE_PROVIDE_OBJECT|DEBUG_BACKTRACE_IGNORE_ARGS);
}

class foo {
  <<__NEVER_INLINE>>
	protected function doCall($dowhat, $how)
:mixed	{
	   HH\dynamic_fun($dowhat)('a','b', $how);
	}
	<<__NEVER_INLINE>>
	static function statCall($dowhat, $how)
:mixed	{
		$obj = new self();
		$obj->doCall($dowhat, $how);
	}
}
<<__EntryPoint>>
function entrypoint_debug_backtrace_options(): void {
  foo::statCall("doit", "debug_print_backtrace");
  foo::statCall("doit", "backtrace_print");
}
