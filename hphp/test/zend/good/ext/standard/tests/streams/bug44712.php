<?php <<__EntryPoint>> function main() {
$ctx = stream_context_get_default();
stream_context_set_params($ctx, array("options" => 1));
}
