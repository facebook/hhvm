<?hh

<<__EntryPoint>>
function foo() :mixed{
	(ZendGoodZendTestsBug43851::$LAST = HH\Lib\Legacy_FIXME\cast_for_arithmetic(ZendGoodZendTestsBug43851::$LAST) + 0) * 1;
	echo "ok\n";
}

abstract final class ZendGoodZendTestsBug43851 {
  public static $LAST;
}
