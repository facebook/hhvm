<?php
function test(){
  //double_encode false
  var_dump(htmlspecialchars('&nbsp;', ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8', false));
  var_dump(htmlspecialchars('<foo> & &nbsp;', ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8', false));
  //double_encode true
  var_dump(htmlspecialchars('&nbsp;', ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8', true));
  var_dump(htmlspecialchars('<foo> & &nbsp;', ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8', true));

}
test();
