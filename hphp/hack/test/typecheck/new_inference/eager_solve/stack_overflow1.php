<?hh

function f(bool $b): void {
  $d = dict[];

  $d[1] = ($b ? shape('a' => dict[]) : dict[]);
  $d[0]['a'] = dict[];
  $x = $d[0]['a']['c'];
  $d[0]['a']['b'] = ($b ? $x : dict[]);
  $d[0]['a']['b']['c'] = 0;

  $d[1] = ($b ? shape('a' => dict[]) : dict[]);
  $d[0]['a'] = dict[];
  $d[0]['a']['b'] = ($b ? $d[0]['a']['c'] : dict[]);
  $d[0]['a']['b']['c'] = 0;

  $d[1] = ($b ? shape('a' => dict[]) : dict[]);
  $d[0]['a'] = dict[];
  $d[0]['a']['b'] = ($b ? $d[0]['a']['c'] : dict[]);
  $d[0]['a']['b']['c'] = 0;
}
