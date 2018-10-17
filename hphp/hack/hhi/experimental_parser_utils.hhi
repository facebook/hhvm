<?hh // decl

namespace HH\ExperimentalParserUtils;

newtype FunctionNode = darray<string, mixed>;
function find_single_function(\HH\ParseTree $json, int $line): ?FunctionNode;
function find_all_functions(\HH\ParseTree $json): dict<int, FunctionNode>;
function body_bounds(FunctionNode $function): ((int, int), (int, int));
