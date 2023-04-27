// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use regex::bytes::RegexSet;

// These VCS paths are already omitted from our Watchman Subscription, but we
// also index the repo without using Watchman in some tools. Hardcode these to
// be filtered away for the non-Watchman use cases.
static VCS_REGEX: &str = r"[.](hg|git|svn)/.*";

static PHPUNIT_RE: &str = r".*flib/intern/third-party/phpunit/phpunit.*\.phpt";

pub struct FilesToIgnore {
    regexes: RegexSet,
}

impl Default for FilesToIgnore {
    fn default() -> Self {
        Self {
            regexes: RegexSet::new([VCS_REGEX, PHPUNIT_RE].into_iter()).unwrap(),
        }
    }
}

impl FilesToIgnore {
    pub fn new(regexes_to_ignore: &[String]) -> Result<Self, regex::Error> {
        Ok(Self {
            regexes: RegexSet::new(
                [VCS_REGEX, PHPUNIT_RE]
                    .into_iter()
                    .chain(regexes_to_ignore.iter().map(String::as_str)),
            )?,
        })
    }

    pub fn should_ignore(&self, path: impl AsRef<Path>) -> bool {
        let path = path.as_ref();
        self.should_ignore_impl(path)
    }

    #[cfg(unix)]
    fn should_ignore_impl(&self, path: &Path) -> bool {
        use std::os::unix::ffi::OsStrExt;
        let path_bytes = path.as_os_str().as_bytes();
        self.regexes.is_match(path_bytes)
    }
}
