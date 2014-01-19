<?php

function foo($a,$b,$c,$d) {
 return implode($a,$b);
 }
function bar($values, $parent_fields) {
  $full_name = implode('___', $parent_fields);
  $body = '';
  $body .= '<div>';
  $body .= '<table id=' . 'bar_' . $full_name . ' border=1>';
  $item_num = 0;
  if (null !== $values) {
    foreach($values as $val) {
      $row_id = 'tr_sentrylist_' . $item_num . '_' . $full_name;
      $body .= '<tr id=' . $row_id . '>';
      $body .= '<td>';
      $body .=  foo($item_num, 0,                    $val, $parent_fields);
      $body .= '</td>';
      $body .= '<td>';
      $body .= foo($item_num, $full_name, 0, 0);
      $body .= '</td>';
      $body .= '</tr>';
      $item_num += 1;
    }
  }
  $body .= '</table>';
}
