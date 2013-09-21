<?php

interface Itest {
	function a();
}

interface Itest2 {
	function a();
}

interface Itest3 extends Itest, Itest2 {
}

echo "done!\n";

?>