<?hh 
<<__EntryPoint>> function main(): void {
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
}
