<?hh

function foo(
): void {
  // hh_log_level('sub', 2);

  $chart_data = Vector {
    Map {
      'y' => 0.,
    },
    Map {
      'y' => 0.,
    },
  }
    ->map(
      $v ==> $v->set(
        'y',
        0.0,
      ),
    )
    ->filter($v ==> {
      // hh_show_env();
      return $v['y'] > 0.0;
    })
    ->map($v ==> $v->set('y', $v['y']));
}
