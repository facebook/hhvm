<?hh <<__EntryPoint>> function main(): void {
var_dump(
    json_encode("1", JSON_NUMERIC_CHECK),
    json_encode("9.4324", JSON_NUMERIC_CHECK),
    json_encode(vec["122321", "3232595.33423"], JSON_NUMERIC_CHECK),
    json_encode("1"),
    json_encode("9.4324"),
    json_encode(vec["122321", "3232595.33423"])
);
}
