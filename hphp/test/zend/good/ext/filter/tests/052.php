<?hh
function filter_cb($var)
:mixed{
  return 1;
}
<<__EntryPoint>> function main(): void {
$data = dict['bar' => vec['fu<script>bar', 'bar<script>fu'] ];
var_dump(filter_var($data, FILTER_SANITIZE_STRING, FILTER_FORCE_ARRAY));
var_dump($data);
}
