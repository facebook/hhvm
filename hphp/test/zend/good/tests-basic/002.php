<?php
parse_str("a=Hello+World", $_POST);

echo $_POST['a']; ?>