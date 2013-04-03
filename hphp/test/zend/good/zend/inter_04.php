<?php 

interface a {
	function b();
}

interface b {
	function b();
}

interface c extends a, b {
}

echo "done!\n";

?>