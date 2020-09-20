<?hh
<<__EntryPoint>> function main(): void {
$json = HH\ffp_parse_string("<?hh type T = shape('a'=>?X, ?'b'=>(function (): Awaitable<?Y>));");
$shape = HH\ExperimentalParserUtils\extract_type_of_only_shape_type_alias($json);
var_dump($shape);
}
