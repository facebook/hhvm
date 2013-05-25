<?php

file_put_contents('testdata.csv', 'eerste;tweede;derde');

$fo = new SplFileObject('testdata.csv');

$fo->rewind( "invalid" );

?>