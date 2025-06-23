#![allow(dead_code)]

use std::path::Component;
use std::path::Path;
use std::path::PathBuf;

pub fn format_path(path: &str) -> String {
    let raw_path = Path::new(path);
    let mut normalized_path = PathBuf::new();

    for component in raw_path.components() {
        match component {
            Component::CurDir => {} // ignore
            Component::ParentDir => {
                if !normalized_path.pop() {
                    // If we can't go up, keep the ..
                    normalized_path.push(component);
                }
            }
            _ => {
                normalized_path.push(component);
            }
        }
    }

    normalized_path.to_string_lossy().to_string()
}

#[cfg(test)]
mod test {
    use anyhow::Result;

    use super::*;

    #[test]
    fn test_format_path() -> Result<()> {
        assert_eq!(format_path("/a/b/c/d/e/f"), "/a/b/c/d/e/f");
        assert_eq!(format_path("/a/b/c/../e"), "/a/b/e");

        assert_eq!(format_path("a/b/c"), "a/b/c");
        assert_eq!(format_path("a/../c"), "c");
        assert_eq!(format_path("a/b/../c_path"), "a/c_path");

        assert_eq!(format_path("a/b/./c"), "a/b/c");
        assert_eq!(format_path("./b/./c/"), "b/c");
        assert_eq!(format_path("/./abc"), "/abc");
        assert_eq!(format_path("a/b/./c/"), "a/b/c"); // does not return directories.

        assert_eq!(format_path("../a/b/./c"), "../a/b/c");
        assert_eq!(format_path("/../abc/"), "/../abc");

        assert_eq!(format_path("a/b/.../c"), "a/b/.../c"); // special characters

        Ok(())
    }
}
