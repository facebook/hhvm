<?hh <<__EntryPoint>> function main(): void {
$context = stream_context_create();

// Single option
var_dump(stream_context_set_option($context, 'http'));

// Array of options
var_dump(stream_context_set_option($context, vec[], 'foo', 'bar'));
}
