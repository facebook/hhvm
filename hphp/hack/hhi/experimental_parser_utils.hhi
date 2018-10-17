<?hh // decl

namespace HH\ExperimentalParserUtils;

newtype FunctionNode = darray<string, mixed>;
function find_single_function(\HH\ParseTree $json, int $line): ?FunctionNode;
function find_all_functions(\HH\ParseTree $json): dict<int, FunctionNode>;
function body_bounds(FunctionNode $function): ((int, int), (int, int));
newtype ShapeNode = darray<string, mixed>;
function find_single_shape_type_alias(\HH\ParseTree $json, string $name): ?(string, ShapeNode);
function extract_shape_comments(ShapeNode $shape): dict<string, vec<string>>;
