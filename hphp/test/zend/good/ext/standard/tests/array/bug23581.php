<?hh <<__EntryPoint>> function main(): void {
var_dump(
  array_map(
    NULL,
    vec[1,2,3],
    vec[4,5,6],
    vec[7,8,9]
  )
);
}
