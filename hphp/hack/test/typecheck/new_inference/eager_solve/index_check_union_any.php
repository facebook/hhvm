////file1.php
<?hh

function any(){
  return dict[];
}

////file2.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function testany():void {
  $x = idx(any(), 'a', dict[]);
  $y = $x['b'];
}
