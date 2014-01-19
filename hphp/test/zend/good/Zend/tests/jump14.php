<?php 

goto A;

{
	B:
		goto C;	
		return;
}

A:
	goto B;



{
	C:
	{
		print "Done!\n";
	}
}

?>