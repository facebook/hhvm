<?php

function foo($params) {
  $listId = (isset($params['id']) && trim($params['id']) != '') ?
    ((int)$params['id'] > 0?(int)$params['id'] :
     ((string)$params['id'] == 'default'?'default': null)):null;
  return $params;
}

var_dump(foo(array('id' => 42)));
