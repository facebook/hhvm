<?php
/*
 * Note: Using error_reporting=E_ALL & ~E_NOTICE to suppress "Trying to get property of non-object" notices.
 */
class StreamExploiter {
	public function stream_close (  ) {
		$doc = new DOMDocument;
		$doc->resolveExternals = true;
		$doc->substituteEntities = true;
		$dir = htmlspecialchars(dirname(getcwd()));
		$dir = str_replace('\\', '/', $dir); // fix for windows
		$doc->loadXML( <<<XML
<!DOCTYPE doc [
	<!ENTITY file SYSTEM "file:///$dir/bad">
]>
<doc>&file;</doc>
XML
		);
		print $doc->documentElement->firstChild->nodeValue;
	}

	public function stream_open (  $path ,  $mode ,  $options ,  &$opened_path ) {
		return true;
	}
}

var_dump(mkdir('test_bug_61367'));
var_dump(mkdir('test_bug_61367/base'));
var_dump(file_put_contents('test_bug_61367/bad', 'blah'));
var_dump(chdir('test_bug_61367/base'));

stream_wrapper_register( 'exploit', 'StreamExploiter' );
$s = fopen( 'exploit://', 'r' );

?>
<?php
unlink('test_bug_61367/bad');
rmdir('test_bug_61367/base');
rmdir('test_bug_61367');
?>