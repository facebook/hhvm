<?hh
<<__EntryPoint>> function main(): void {
foreach (function(){ return 1; } as $y) { 
	var_dump($y);	
}

print "ok\n";
}
