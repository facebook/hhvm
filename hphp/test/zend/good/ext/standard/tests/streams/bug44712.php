<?hh <<__EntryPoint>> function main(): void {
$ctx = stream_context_get_default();
stream_context_set_params($ctx, darray["options" => 1]);
}
