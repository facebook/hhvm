<?hh

class MyClass
{
  function __toString() {
    return "MyClass";
  }
}
<<__EntryPoint>>
function entrypoint_image_type_to_mime_type_variation1(): void {
  /* Prototype  : string image_type_to_mime_type(int imagetype)
   * Description: Get Mime-Type for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype 
   * Source code: ext/standard/image.c
   */

  echo "*** Testing image_type_to_mime_type() : usage variations ***\n";

  error_reporting(E_ALL ^ E_NOTICE);

  //get an unset variable
  $unset_var = 10;
  unset ($unset_var);

  //array of values to iterate over
  $values = varray[

        // float data
        100.5,
        -100.5,
        100.1234567e10,
        100.7654321E-10,
        .5,

        // array data
        varray[],
        darray['color' => 'red', 'item' => 'pen'],

        // null data
        NULL,
        null,

        // boolean data
        true,
        false,
        TRUE,
        FALSE,

        // empty data
        "",
        '',

        // string data
        "string",
        'string',

        // object data
        new MyClass(),

        // undefined data
        @$undefined_var,

        // unset data
        @$unset_var,
  ];

  // loop through each element of the array for imagetype
  $iterator = 1;
  foreach($values as $value) {
        echo "\n-- Iteration $iterator --\n";
        try { var_dump( image_type_to_mime_type($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
        $iterator++;
  }
  echo "===DONE===\n";
}
