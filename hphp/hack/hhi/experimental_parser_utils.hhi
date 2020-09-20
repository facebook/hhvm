<?hh

namespace HH\ExperimentalParserUtils;

newtype FunctionNode = darray<string, mixed>;
function find_all_functions(\HH\ParseTree $json): dict<int, FunctionNode>;
function body_bounds(FunctionNode $function): ((int, int), (int, int));
newtype ShapeNode = darray<string, mixed>;
function find_single_shape_type_alias(\HH\ParseTree $json, string $name): ?(string, ShapeNode);
function extract_shape_comments(ShapeNode $shape): dict<string, vec<string>>;
newtype ClassBodyNode = darray<string, mixed>;
function find_class_body(\HH\ParseTree $json, string $name): ?ClassBodyNode;
function find_class_shape_type_constant(ClassBodyNode $class_body, string $name): ?ShapeNode;
function find_class_method_shape_return_type(ClassBodyNode $class_body, string $name): ?ShapeNode;
newtype MethodParametersNode = darray<string, mixed>;
function find_method_parameters(\HH\ParseTree $json, string $method_name, int $line_number): MethodParametersNode;
function extract_parameter_comments(MethodParametersNode $params): dict<string, vec<string>>;
newtype EnumBodyNode = darray<string, mixed>;
function find_enum_body(\HH\ParseTree $json, string $name): ?EnumBodyNode;
function extract_enum_comments(EnumBodyNode $enumerators): dict<string, vec<string>>;
function extract_type_of_only_shape_type_alias(\HH\ParseTree $json): dict<string, (string, bool)>;
