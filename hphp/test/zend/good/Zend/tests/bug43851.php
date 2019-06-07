<?hh

<<__EntryPoint>>
function foo() {
	(ZendGoodZendTestsBug43851::$LAST = ZendGoodZendTestsBug43851::$LAST + 0) * 1;
	echo "ok\n";
}

abstract final class ZendGoodZendTestsBug43851 {
  public static $LAST;
}
