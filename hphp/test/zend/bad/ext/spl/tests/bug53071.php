<?php
gc_enable();
class myClass
{
	public $member;
}
function LimitedScope()
{
	$myA = new myClass();
	$myB = new SplObjectStorage();
	$myC = new myClass();
	$myC->member = $myA; // myC has a referece to myA
	$myB->Attach($myC);  // myB attaches myC
	$myA->member = $myB; // myA has myB, comleting the cycle
}
LimitedScope();
var_dump(gc_collect_cycles());

echo "Done.\n";

?>
