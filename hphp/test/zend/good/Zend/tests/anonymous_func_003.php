<?hh 
<<__EntryPoint>> function main(): void {
try {
  $a = () ==> new Exception('test');
	throw $a();
} catch (Exception $e) {
	var_dump($e->getMessage() == 'test');
}
}
