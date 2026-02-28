<?hh
<<__EntryPoint>> function main(): void {
foreach (vec['2006-05-13', '06-12-12', 'data: "12-Aug-87"'] as $s) {
    $m = null;
    var_dump(preg_match_with_matches(
      '~
		(?P<date>
		(?P<year>(\d{2})?\d\d) -
		(?P<month>(?:\d\d|[a-zA-Z]{2,3})) -
		(?P<day>[0-3]?\d))
	~x',
      $s,
      inout $m,
    ));

    var_dump($m);
}
}
