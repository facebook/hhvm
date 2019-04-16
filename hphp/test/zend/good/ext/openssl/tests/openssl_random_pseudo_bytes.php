<?php
for ($i = 0; $i < 10; $i++) {
	var_dump(bin2hex((string)openssl_random_pseudo_bytes($i, &$strong)));
}

