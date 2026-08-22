// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::borrow::Cow;
use std::ops::Range;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::LazyLock;

use rc_pos::Pos;
use relative_path::Prefix;
use relative_path::RelativePath;
use toml::Spanned;

use crate::r#gen::package::Package;
use crate::r#gen::package::PosId;
use crate::r#gen::package_info::PackageInfo;

pub type Errors = Vec<(Pos, String, Vec<(Pos, String)>)>;

pub fn package_info_to_vec(
    filename: &str,
    info: packages::PackageInfo,
) -> Result<Vec<Package>, Errors> {
    let pos_from_span = |span: (usize, usize)| {
        let (start_offset, end_offset) = span;
        let start_lnum = info.line_number(start_offset);
        let start_bol = info.beginning_of_line(start_lnum);
        let end_lnum = info.line_number(end_offset);
        let end_bol = info.beginning_of_line(end_lnum);

        Pos::from_lnum_bol_offset(
            Arc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(filename))),
            (start_lnum, start_bol, start_offset),
            (end_lnum, end_bol, end_offset),
        )
    };
    let errors = info
        .errors()
        .iter()
        .map(|e| {
            let pos = pos_from_span(e.span());
            let msg = e.msg();
            let reasons = e
                .reasons()
                .into_iter()
                .map(|(start, end, reason)| (pos_from_span((start, end)), reason))
                .collect();
            (pos, msg, reasons)
        })
        .collect::<Errors>();
    if !errors.is_empty() {
        return Err(errors);
    };
    let convert = |x: &Spanned<String>| -> PosId {
        let Range { start, end } = x.span();
        let pos = pos_from_span((start, end));
        let id = x.to_owned().into_inner();
        PosId(pos, id)
    };
    let convert_many = |xs: &Option<packages::NameSet>| -> Vec<PosId> {
        xs.as_ref()
            .unwrap_or_default()
            .iter()
            .map(convert)
            .collect()
    };

    let mut packages: Vec<Package> = info
        .packages()
        .iter()
        .map(|(name, package)| Package {
            name: convert(name),
            includes: convert_many(&package.includes),
            soft_includes: convert_many(&package.soft_includes),
            include_paths: convert_many(&package.include_paths),
            enable_strict_isolation: package.enable_strict_isolation,
            is_implicit: false,
        })
        .collect();

    // Append one entry per implicit-package family. The family's `path` becomes
    // its sole include_path and the entry is flagged `is_implicit`. Members
    // `F.D` are synthesized lazily during lookup (see `package_info.ml` and
    // `PackageInfo::get_package_for_file`), never materialized here.
    for (name, fam) in info.implicit_packages().iter() {
        packages.push(Package {
            name: convert(name),
            includes: convert_many(&fam.includes),
            soft_includes: convert_many(&fam.soft_includes),
            include_paths: vec![convert(&fam.path)],
            // Implicit packages opt into strict isolation.
            enable_strict_isolation: true,
            is_implicit: true,
        });
    }
    Ok(packages)
}

/// Synthesize the member package `F.D` of an implicit family. `family` is the
/// flagged family entry (its single `include_path` is the family `path`);
/// `member_dir` is the first path segment `D` below that path. This is a pure
/// function of the family declaration and `D` — it touches no filesystem.
fn synthesize_member(family: &Package, member_dir: &str) -> Package {
    let family_name = &family.name;
    let member_name = format!("{}.{}", family_name.1, member_dir);
    // The family's include_path is its `path`; the member's is `path/D/`.
    let (member_path_pos, member_path) = match family.include_paths.first() {
        Some(PosId(pos, path)) => (pos.clone(), format!("{}{}/", path, member_dir)),
        // A well-formed family always has exactly one include_path; fall back
        // defensively to the family name's position.
        None => (family_name.0.clone(), format!("{}/", member_dir)),
    };
    Package {
        name: PosId(family_name.0.clone(), member_name),
        includes: family.includes.clone(),
        soft_includes: family.soft_includes.clone(),
        include_paths: vec![PosId(member_path_pos, member_path)],
        // Members inherit the family's strict-isolation setting.
        enable_strict_isolation: family.enable_strict_isolation,
        is_implicit: true,
    }
}

impl TryFrom<packages::PackageInfo> for PackageInfo {
    type Error = Errors;
    fn try_from(info: packages::PackageInfo) -> Result<Self, Errors> {
        let result = package_info_to_vec("PACKAGES.toml", info);
        match result {
            Ok(packages) => {
                let existing_packages = packages
                    .iter()
                    .map(|package| (package.name.1.clone(), package.clone()))
                    .collect();
                // Build include_path_to_package_map, sorted by anti-lexicographic include_path
                // order so a simple linear search returns the most precise path that includes
                // a given file.
                let mut include_path_pairs: Vec<_> = packages
                    .iter()
                    .flat_map(|package| {
                        package
                            .include_paths
                            .iter()
                            .map(move |include_path| (include_path.1.clone(), package.clone()))
                    })
                    .collect();
                include_path_pairs.sort_by(|(a, _), (b, _)| b.cmp(a));
                let include_path_to_package_map = include_path_pairs.into_iter().collect();
                Ok(PackageInfo {
                    existing_packages,
                    include_path_to_package_map,
                })
            }
            Err(err) => Err(err),
        }
    }
}

/// Strips the container prefix from a multifile test path
/// `<container>--<simulated/path.php>`. The simulated path is repo-relative, so
/// the whole container path goes, hence anchored and greedy.
///
/// Must stay identical to `Multifile.strip_multifile_prefix` in
/// `utils/multifile.ml`, or a file resolves to different packages in OCaml and
/// Rust.
static MULTIFILE_PREFIX: LazyLock<regex::Regex> = LazyLock::new(|| {
    regex::Regex::new(r"^.*--").expect("should compile: the pattern is a literal")
});

/// Where a file sits relative to the implicit package families. The illegal
/// placements carry the family's positioned name, so the caller can point at its
/// declaration in PACKAGES.toml.
pub enum ImplicitFamilyPlacement<'a> {
    /// Belongs to a member package, or is not under any family at all.
    Valid,
    /// Lies directly under a family `path` with no member directory in between.
    DirectlyUnderFamily(&'a PosId),
    /// Lies under a directory whose name is not a valid Hack identifier, so that
    /// directory names no member package. Carries the offending name.
    InvalidMemberDir(&'a PosId, String),
}

/// The path rewriting every package lookup applies before prefix-matching. All
/// lookups must go through this, so they prefix-match the same string.
fn normalize(support_multifile_tests: bool, path: &str) -> Cow<'_, str> {
    if support_multifile_tests {
        MULTIFILE_PREFIX.replace(path, "")
    } else {
        Cow::Borrowed(path)
    }
}

impl PackageInfo {
    /// The most precise package whose `include_path` prefixes `path`, together
    /// with the part of `path` lying below that `include_path`.
    fn match_include_path<'a, 'p>(&'a self, path: &'p str) -> Option<(&'a Package, &'p str)> {
        let (include_path, package) = self
            .include_path_to_package_map
            .iter()
            .find(|(include_path, _)| path.starts_with(include_path))?;
        Some((package, &path[include_path.len()..]))
    }

    pub fn get_package_for_file(
        &self,
        support_multifile_tests: bool,
        path: &str,
    ) -> Option<Cow<'_, Package>> {
        let path = normalize(support_multifile_tests, path);
        let (package, remainder) = self.match_include_path(&path)?;
        if !package.is_implicit {
            // Common case: the stored package is returned by reference, no clone.
            return Some(Cow::Borrowed(package));
        }
        // The member directory `D` is the first segment after the family
        // `path`. Only child directories denote members, so a file lying
        // directly in the family path belongs to no package.
        match remainder.split_once('/') {
            Some((dir, _)) if !dir.is_empty() => Some(Cow::Owned(synthesize_member(package, dir))),
            _ => None,
        }
    }

    /// Where `path` sits relative to the implicit package families.
    ///
    /// - `Valid` -- belongs to a member package, or lies under no family at all.
    /// - `DirectlyUnderFamily` -- lies directly under a family `path` with no
    ///   member directory in between, e.g. `prototypes/loose.php` under a family
    ///   declared at `prototypes/`.
    /// - `InvalidMemberDir` -- the member directory is not a valid Hack
    ///   identifier, so it names no member package.
    pub fn check_implicit_family_placement(
        &self,
        support_multifile_tests: bool,
        path: &str,
    ) -> ImplicitFamilyPlacement<'_> {
        let path = normalize(support_multifile_tests, path);
        let Some((package, remainder)) = self.match_include_path(&path) else {
            return ImplicitFamilyPlacement::Valid;
        };
        if !package.is_implicit {
            return ImplicitFamilyPlacement::Valid;
        }
        match remainder.split_once('/') {
            Some((dir, _)) if hack_name::is_valid_identifier(dir) => ImplicitFamilyPlacement::Valid,
            Some((dir, _)) if !dir.is_empty() => {
                ImplicitFamilyPlacement::InvalidMemberDir(&package.name, dir.to_owned())
            }
            _ => ImplicitFamilyPlacement::DirectlyUnderFamily(&package.name),
        }
    }
}

#[cfg(test)]
mod test {
    use rc_pos::Pos;

    use super::*;

    fn pos_id(s: &str) -> PosId {
        PosId(Pos::NONE, s.to_string())
    }

    // Exercises the lazy member-synthesis algorithm directly (the Rust mirror of
    // the OCaml `Package_info.get_package_for_file`). No member package is stored
    // in the map -- only the flagged family entry -- so any returned member must
    // have been synthesized on demand from the queried path.
    #[test]
    fn lazy_member_synthesis() {
        let family = Package {
            name: pos_id("prototypes"),
            includes: vec![pos_id("intern")],
            soft_includes: vec![],
            include_paths: vec![pos_id("www/prototypes/")],
            enable_strict_isolation: true,
            is_implicit: true,
        };
        let info = PackageInfo {
            existing_packages: Default::default(),
            include_path_to_package_map: vec![("www/prototypes/".to_string(), family)],
        };

        // A file in a child directory `alpha` resolves to the synthesized member
        // `prototypes.alpha`, inheriting the family's includes and getting its
        // own derived include_path.
        let member = info
            .get_package_for_file(false, "www/prototypes/alpha/a.php")
            .expect("file under a child directory should belong to a member package");
        assert_eq!(member.name.1, "prototypes.alpha");
        assert_eq!(member.include_paths[0].1, "www/prototypes/alpha/");
        assert_eq!(member.includes[0].1, "intern");
        assert!(member.is_implicit);
        // Members inherit the family's strict-isolation setting.
        assert!(member.enable_strict_isolation);

        // A deeper file resolves to the same first-segment member.
        assert_eq!(
            info.get_package_for_file(false, "www/prototypes/alpha/sub/deep.php")
                .unwrap()
                .name
                .1,
            "prototypes.alpha"
        );

        // A file lying directly in the family path (no child directory) belongs
        // to no member.
        assert!(
            info.get_package_for_file(false, "www/prototypes/top.php")
                .is_none()
        );

        // A file outside the family path belongs to no package.
        assert!(info.get_package_for_file(false, "other/x.php").is_none());

        // Resolution does NOT reject a badly-named member directory; it
        // synthesizes a member like any other.
        for bad in [
            "www/prototypes/proto.v1/a.php",
            "www/prototypes/1bad/a.php",
            "www/prototypes/:xhp/a.php",
        ] {
            assert!(
                info.get_package_for_file(false, bad).is_some(),
                "{bad} should still resolve; rejecting it is the placement check's job"
            );
        }
    }

    // Identifier validity is decided only by the placement classifier.
    #[test]
    fn placement_rejects_badly_named_member_dirs() {
        let family = Package {
            name: pos_id("prototypes"),
            includes: vec![],
            soft_includes: vec![],
            include_paths: vec![pos_id("www/prototypes/")],
            enable_strict_isolation: true,
            is_implicit: true,
        };
        let info = PackageInfo {
            existing_packages: Default::default(),
            include_path_to_package_map: vec![("www/prototypes/".to_string(), family)],
        };

        for (path, expected_dir) in [
            ("www/prototypes/proto.v1/a.php", "proto.v1"),
            ("www/prototypes/1bad/a.php", "1bad"),
            ("www/prototypes/:xhp/a.php", ":xhp"),
        ] {
            match info.check_implicit_family_placement(false, path) {
                ImplicitFamilyPlacement::InvalidMemberDir(_, dir) => {
                    assert_eq!(dir, expected_dir, "wrong directory reported for {path}")
                }
                _ => panic!("{path} should be reported as a badly-named member directory"),
            }
        }

        // A validly-named directory, and a file outside any family, are fine.
        for ok in ["www/prototypes/alpha/a.php", "other/x.php"] {
            assert!(matches!(
                info.check_implicit_family_placement(false, ok),
                ImplicitFamilyPlacement::Valid
            ));
        }
    }

    // A container path with directories in it must be dropped whole; a
    // `/`-restricted pattern would leave `test/pkg/` glued to the front.
    #[test]
    fn multifile_prefix_strips_the_whole_container_path() {
        let family = Package {
            name: pos_id("prototypes"),
            includes: vec![],
            soft_includes: vec![],
            include_paths: vec![pos_id("www/prototypes/")],
            enable_strict_isolation: true,
            is_implicit: true,
        };
        let info = PackageInfo {
            existing_packages: Default::default(),
            include_path_to_package_map: vec![("www/prototypes/".to_string(), family)],
        };

        for path in [
            "container.php--www/prototypes/alpha/a.php",
            "test/pkg/container.php--www/prototypes/alpha/a.php",
        ] {
            assert_eq!(
                info.get_package_for_file(true, path)
                    .unwrap_or_else(|| panic!("{path} should resolve to a member package"))
                    .name
                    .1,
                "prototypes.alpha"
            );
        }

        // Without the flag the path is taken literally, so the mangled name does
        // not match the family.
        assert!(
            info.get_package_for_file(false, "container.php--www/prototypes/alpha/a.php")
                .is_none()
        );
    }
}
