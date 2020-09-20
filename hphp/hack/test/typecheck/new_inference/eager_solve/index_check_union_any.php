////file1.php
<?hh // partial

function any(){
  return darray[];
}

////file2.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function testany():void {
  $x = idx(any(), 'a', darray[]);
  $y = $x['b'];
}
