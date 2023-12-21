<?hh
<<__EntryPoint>> function main(): void {
$file_paths = vec[
  /* simple paths (forward slashes) */
  "bar",
  "/foo/bar",
  "foo/bar",
  "/bar",

  /* simple paths with trailing slashes (forward slashes) */
  "bar/",
  "/bar/",
  "/foo/bar/",
  "foo/bar/",
  "/bar/",

  /* simple paths (backslashes) */
  "bar",
  "\\foo\\bar",
  "foo\\bar",
  "\\bar",

  /* simple paths with trailing slashes (backslashes) */
  "bar\\",
  "\\bar\\",
  "\\foo\\bar\\",
  "foo\\bar\\",
  "\\bar\\",

  /* paths with numeric strings */
  "10.5\\10.5",
  "10.5/10.5",
  "10.5",
  "105",
  "/10.5",
  "\\10.5",
  "10.5/",
  "10.5\\",
  "10/10.zip",
  "0",
  '0',

  /* path with spaces */
  " ",
  ' ',

  /* empty paths */
  "",
  '',
];

foreach ($file_paths as $file_path) {
	var_dump(basename($file_path));
}
}
