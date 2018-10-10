<?hh

class FQLStatement {};

class FQLFieldExpression {};

class FQLConstantExpression {};

class FQLComparisonExpression {};

<<__EntryPoint>>
function main() {
  fql_set_static_data_10(array('user'), array());
  var_dump(fql_multiparse_10(array(0 => 'select name from user where uid=4')));
}
