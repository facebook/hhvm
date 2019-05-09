<?php
<<__EntryPoint>> function main() {
foreach (function(){ return 1; } as $y) { 
	var_dump($y);	
}

print "ok\n";
}
