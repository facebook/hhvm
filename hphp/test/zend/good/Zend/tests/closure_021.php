<?hh
<<__EntryPoint>> function main() {
$foo = function() {
	try {
		throw new Exception('test!');
	} catch(Exception $e) {
		throw $e;
	}
};

try {
	$foo();
} catch (Exception $e) {
	var_dump($e->getMessage());
}
}
