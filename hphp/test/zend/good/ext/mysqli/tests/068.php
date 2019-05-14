<?php <<__EntryPoint>> function main() {
$s = mysqli_get_client_info();
echo gettype($s);
}
