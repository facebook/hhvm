<?hh

type X = shape(
  'some_int' => int, // Int description
  /* BEGIN MANUAL SECTION words words */
  'no_description' => int,
  // Float description
  'some_float' => float,
  // Multiple
  //
  // paragraphs
  'some_string' => string, /*

  Another paragraph.*/

  // Long

  /*     long    */ //description
  //
  // Paragraph 2
  /*
  */
  'some_bool' /* Paragraph */ =>//...
    bool/*3*//**///... still 3
  ,
  #hello
  'pound_comment' => int,
  /**
   * comment
   */
  'star_comment' => int,
  ...
);
<<__EntryPoint>> function main(): void {
$program = file_get_contents(__FILE__);
$json = HH\ffp_parse_string($program);
$result = HH\ExperimentalParserUtils\find_single_shape_type_alias($json, "x"); // case insensitive
invariant($result !== null, "Failed to find shape type alias");
list($real_name, $shape) = $result;
var_dump(HH\ExperimentalParserUtils\extract_shape_comments($shape));
}
