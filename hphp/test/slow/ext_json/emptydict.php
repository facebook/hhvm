<?hh


// Copyright 2004-2015 Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_emptydict() {
var_dump(json_decode(
           <<<EOT
{
   "staticBitmask": {
}
,
   "spaceId": 3
}
EOT
           , true));
}
