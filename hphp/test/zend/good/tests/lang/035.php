<?php
class MyException extends Exception {
	function MyException($_error) {
		$this->error = $_error;	
	}
	
	function getException()
	{
		return $this->error;	
	}
}

function ThrowException()
{
	throw new MyException("'This is an exception!'");	
}


try {
} catch (MyException $exception) {
	print "There shouldn't be an exception: " . $exception->getException();
	print "\n";
}

try {
	ThrowException();	
} catch (MyException $exception) {
	print "There was an exception: " . $exception->getException();
	print "\n";
}
?>