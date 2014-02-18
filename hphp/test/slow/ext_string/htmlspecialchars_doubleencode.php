<?php
function test(){
  var_dump(htmlspecialchars('&nbsp;', ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8', false));
  var_dump(htmlspecialchars('<foo> & &nbsp;', ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8', false));
}
test();
