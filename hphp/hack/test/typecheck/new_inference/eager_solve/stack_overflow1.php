<?hh // strict

function f(bool $b): void {
  $d = darray[];

  $d[1] = ($b ? shape('a' => darray[]) : darray[]);
  $d[0]['a'] = darray[];
  $x = $d[0]['a']['c'];
  $d[0]['a']['b'] = ($b ? $x : darray[]);
  $d[0]['a']['b']['c'] = 0;

  $d[1] = ($b ? shape('a' => darray[]) : darray[]);
  $d[0]['a'] = darray[];
  $d[0]['a']['b'] = ($b ? $d[0]['a']['c'] : darray[]);
  $d[0]['a']['b']['c'] = 0;

  $d[1] = ($b ? shape('a' => darray[]) : darray[]);
  $d[0]['a'] = darray[];
  $d[0]['a']['b'] = ($b ? $d[0]['a']['c'] : darray[]);
  $d[0]['a']['b']['c'] = 0;
}
