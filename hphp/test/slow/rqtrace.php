<?hh

<<__EntryPoint>>
function main() :mixed{
  __hhvm_intrinsics\rqtrace_create_event("EVENT0", 100, 200, dict[]);
  __hhvm_intrinsics\rqtrace_create_event("EVENT1", 200, 300, dict['a'=>'b']);
  __hhvm_intrinsics\rqtrace_create_scope("SCOPE0", 300, 400, dict[]);
  __hhvm_intrinsics\rqtrace_create_scope("SCOPE1", 400, 500, dict['c'=>'d']);

  __hhvm_intrinsics\rqtrace_create_scoped_events(
    "OUTER_SCOPE",
    600,
    1200,
    "ALPHA_",
    "_OMEGA",
    dict['scope_attr0' => 'foo', 'scope_attr1' => 'bar'],
    dict[
      'INNER_EVENT0' => tuple(650, 700, dict['e0' => 'a', 'e1' => 'b']),
      'INNER_EVENT1' => tuple(700, 750, dict['e2' => 'a', 'e3' => 'b']),
      'INNER_EVENT2' => tuple(750, 800, dict['e4' => 'a', 'e5' => 'b']),
      'INNER_EVENT3' => tuple(800, 850, dict['e6' => 'a', 'e7' => 'b']),
    ]
  );

  __hhvm_intrinsics\rqtrace_create_event("EVENT1", 850, 900, dict['e'=>'f']);
  __hhvm_intrinsics\rqtrace_create_scope("SCOPE1", 900, 950, dict['g'=>'h']);

  $valid = vec[
    'EVENT0',
    'EVENT1',
    'SCOPE0',
    'SCOPE1',
    'ALPHA_INNER_EVENT0_OMEGA',
    'ALPHA_INNER_EVENT1_OMEGA',
    'ALPHA_INNER_EVENT2_OMEGA',
    'ALPHA_INNER_EVENT3_OMEGA',
  ];

  $a = array_intersect(array_keys(HH\rqtrace\all_request_stats()), $valid);
  sort(inout $a);
  var_dump($a);

  var_dump(HH\rqtrace\request_event_stats('EVENT0'));
  var_dump(HH\rqtrace\request_event_stats('EVENT1'));
  var_dump(HH\rqtrace\request_event_stats('ALPHA_INNER_EVENT0_OMEGA'));
  var_dump(HH\rqtrace\request_event_stats('ALPHA_INNER_EVENT1_OMEGA'));
  var_dump(HH\rqtrace\request_event_stats('ALPHA_INNER_EVENT2_OMEGA'));
  var_dump(HH\rqtrace\request_event_stats('ALPHA_INNER_EVENT3_OMEGA'));
}
