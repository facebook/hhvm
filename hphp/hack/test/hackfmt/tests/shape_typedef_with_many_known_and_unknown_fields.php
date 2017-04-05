<?hh // strict

type ShapeWithKnownAndUnknownFields = shape( 'field1' => int, 'field2' => int,
'field3' => int, 'field4' => int, 'field5' => int,
    'field6' => int, 'field7' => int, ...);
