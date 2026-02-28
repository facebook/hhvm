<?hh

<<__EntryPoint>>
function main(): void {
  // Get the autoloader to initialize
  HH\autoload_is_native();

  try{
    var_dump(HH\FileDecls::parseTypeExpression('5'));
  } catch (Exception $ex) {
    var_dump($ex->getMessage());
  }

  var_dump(HH\FileDecls::parseTypeExpression('int'));

  $complex = HH\FileDecls::parseTypeExpression('shape(
    ?"first" => string,
    "second" => ?dict<int, shape("other" => int)>,
  )');
  var_dump($complex);


  $shape_fields = $complex['subtypes'] ?? vec[];
  if (count($shape_fields) !== 2) {
    echo "Incorrect shape field count\n";
    return;
  }
  foreach ($shape_fields as $field) {
    echo "Shape field: ".($field['name']??'(missing name)')."\n";
    echo "It is of type ".$field['type']['type']." and it is ".(
     ($field['is_optional'] ?? false) ? "optional" : "not optional"
    )."\n";
  }
}
