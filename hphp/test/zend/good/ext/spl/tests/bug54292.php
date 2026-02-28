<?hh
<<__EntryPoint>> function main(): void {
try {
	new SplFileObject('foo', vec[]);
} catch (Exception $e) {
	var_dump($e->getMessage());
}
}
