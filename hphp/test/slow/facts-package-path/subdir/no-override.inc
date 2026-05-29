<?hh
// No __PackageOverride — package should be resolved from PACKAGES.toml via
// path-based matching. The path "subdir/no-override.inc" matches the
// include_path "//subdir/" (package "a"), which is more specific than
// the catch-all "//" (package "fallthrough").
class NoOverride {}
