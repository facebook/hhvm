<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$keyset = keyset [ ] ;
$keyset = keyset [ 1 ] ;
$keyset = keyset [ '1' ] ;
$keyset = keyset [ '1', 2 ] ;
$keyset = keyset [ keyset [ keyset [] ] ];

$keyset = keyset ( );
$keyset = keyset ( $keyset );

foo() -> keyset ( );
foo() -> keyset ( $keyset );

Foo::keyset ( );
Foo::keyset ( $keyset );

$keyset = Keyset\keys ( $keyset );
