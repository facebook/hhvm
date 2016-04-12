<?hh

// ensure legacy token name is returned
var_dump(token_name(T_DOUBLE_COLON));

// see also hphp/hack/hhi/stdlib/builtins_tokens.php
$token_range = range(1, 1000);

$undefined_tokens = array();
$mismatches = array();

$expected_exceptions = array(
  'T_XHP_LABEL' => true,
  'T_XHP_TEXT' => true,
  'T_XHP_ATTRIBUTE' => true,
  'T_XHP_CATEGORY' => true,
  'T_XHP_CATEGORY_LABEL' => true,
  'T_XHP_CHILDREN' => true,
  'T_ENUM' => true,
  'T_XHP_REQUIRED' => true,
  'T_XHP_TAG_LT' => true,
  'T_XHP_TAG_GT' => true,
  'T_HH_ERROR' => true,
  'T_TYPELIST_LT' => true,
  'T_TYPELIST_GT' => true,
  'T_UNRESOLVED_LT' => true,
  'T_COLLECTION' => true,
  'T_SHAPE' => true,
  'T_TYPE' => true,
  'T_UNRESOLVED_TYPE' => true,
  'T_NEWTYPE' => true,
  'T_UNRESOLVED_NEWTYPE' => true,
  'T_AWAIT' => true,
  'T_ASYNC' => true,
  'T_LAMBDA_ARROW' => true,
  'T_LAMBDA_OP' => true,
  'T_LAMBDA_CP' => true,
  'T_NULLSAFE_OBJECT_OPERATOR' => true,
  'T_UNRESOLVED_OP' => true,
  'T_HASHBANG' => true,
  'T_SUPER' => true,
);

foreach ($token_range as $potential_token) {
  $token_name = token_name($potential_token);
  if ($token_name === 'UNKNOWN') {
    continue;
  }

  if (isset($expected_exceptions[$token_name])) {
    continue;
  }

  if (!defined($token_name)) {
    $undefined_tokens[$potential_token] = $token_name;
    continue;
  }

  if (constant($token_name) !== $potential_token) {
    $mismatches[$token_name] = Pair {constant($token_name), $potential_token};
  }
}

echo 'Undefined but name unexpectedly returned by token_name:', "\n";
var_dump($undefined_tokens);
echo 'token_name result disagrees with constant value:', "\n";
var_dump($mismatches);
