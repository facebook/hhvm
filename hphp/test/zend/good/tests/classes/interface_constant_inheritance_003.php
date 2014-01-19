<?php
interface I1 {
	const FOO = 10;
}

interface I2 {
	const FOO = 10;
}

class C implements I1,I2 {
}

echo "Done\n";
?>