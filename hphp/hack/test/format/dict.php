<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$dict = dict [ ] ;
$dict = dict [ 1=>1 ] ;
$dict = dict [ '1'=>'1' ] ;
$dict = dict [ '1'=>'1',2=>2 ] ;
$dict = dict [ dict [ dict [] ] ];

$dict = dict ( );
$dict = dict ( $dict );

foo() -> dict ( );
foo() -> dict ( $dict );

Foo::dict ( );
Foo::dict ( $dict );

$dict = Dict\count_values ( $dict );
