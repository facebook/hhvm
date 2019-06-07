<?hh 
<<__EntryPoint>> function main() {
goto foo;

try {
	print 1;
	
	foo:
	print 2;
} catch (Exception $e) {
	
}
}
