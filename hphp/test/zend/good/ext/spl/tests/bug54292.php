<?hh
<<__EntryPoint>> function main(): void {
try {
	new SplFileObject('foo', varray[]);
} catch (Exception $e) {
	var_dump($e->getMessage());
}
}
