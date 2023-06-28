<?hh

function backtrace_print($opt = null)
:mixed{
	if(is_null($opt)) {
		print_r(debug_backtrace());
	} else {
		print_r(debug_backtrace($opt));
	}
	__hhvm_intrinsics\launder_value($opt);
}

function doit($a, $b, $how)
:mixed{
	echo "==default\n";
	$how();
	echo "==1\n";
	$how(1);
	echo "==0\n";
	$how(0);
	echo "==DEBUG_BACKTRACE_PROVIDE_OBJECT\n";
	$how(DEBUG_BACKTRACE_PROVIDE_OBJECT);
	echo "==DEBUG_BACKTRACE_IGNORE_ARGS\n";
	$how(DEBUG_BACKTRACE_IGNORE_ARGS);
	echo "==both\n";
	$how(DEBUG_BACKTRACE_PROVIDE_OBJECT|DEBUG_BACKTRACE_IGNORE_ARGS);
}

class foo {
  <<__NEVER_INLINE>>
	protected function doCall($dowhat, $how)
:mixed	{
	   $dowhat('a','b', $how);
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
