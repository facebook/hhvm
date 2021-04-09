<?hh

function prim(int $i): void {}

function call_prim(float $f, ?int $x): void {
    /* HH_FIXME[4110] */
    prim($f); /* call_single.php:7:10 */

    /* HH_FIXME[4110] */
    prim($x); /* call_single.php:10:10 */
}
