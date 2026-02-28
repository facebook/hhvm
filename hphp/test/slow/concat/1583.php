<?hh


<<__EntryPoint>>
function main_1583() :mixed{
$a = vec[1, vec[1, vec[1]]];
$a[1][1][] = 3;
var_dump($a);
$a[1][1][1] = "1";
$a[1][1][1] .= "2";
$a[1][1][1] .= "3";
$a[1][1][1] .= "4";
$a[1][1][1] .= "5";
var_dump($a);
$payload = dict[];
$payload['pane_html'] = null;
$payload['pane_html'] = '<div id="beacon_accepted_pane" class="beacon_status_pane" style="display: none">';
$payload['pane_html'] .= '<div class="beacon_status_message">';
}
