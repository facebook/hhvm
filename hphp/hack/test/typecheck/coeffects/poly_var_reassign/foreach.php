<?hh

abstract class V { abstract const ctx C; }

function foreach_v(V $v)[$v::C]: void {
  foreach (vec[] as $v) {}}

function foreach_kv_l(V $v)[$v::C]: void {
  foreach (dict[] as $v => $_) {}}

function foreach_kv_r(V $v)[$v::C]: void {
  foreach (dict[] as $_ => $v) {}}

//
