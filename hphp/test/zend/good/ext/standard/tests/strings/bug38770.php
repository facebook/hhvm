<?php
<<__EntryPoint>> function main() {
foreach (array('N','l') as $v) {
	print_r(unpack($v, pack($v, -30000)));
}

echo "Done\n";
}
