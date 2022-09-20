/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/scm/SCM.h"
#include <memory>
#include "watchman/Logging.h"
#include "watchman/fs/FileInformation.h"
#include "watchman/scm/Git.h"
#include "watchman/scm/Mercurial.h"

namespace watchman {

static const w_string_piece kGit{".git"};
static const w_string_piece kHg{".hg"};

SCM::~SCM() {}

SCM::SCM(w_string_piece rootPath, w_string_piece scmRoot)
    : rootPath_(rootPath.asWString()), scmRoot_(scmRoot.asWString()) {}

const w_string& SCM::getRootPath() const {
  return rootPath_;
}

const w_string& SCM::getSCMRoot() const {
  return scmRoot_;
}

std::optional<w_string> findFileInDirTree(
    w_string_piece rootPath,
    std::initializer_list<w_string_piece> candidates) {
  w_check(
      rootPath.pathIsAbsolute(), "rootPath must be absolute: ", rootPath, "\n");

  while (true) {
    for (auto& candidate : candidates) {
      auto path = w_string::pathCat({rootPath, candidate});

      if (w_path_exists(path.c_str())) {
        return std::move(path);
      }
    }

    auto next = rootPath.dirName();
    if (next == rootPath) {
      // We can't go any higher, so we couldn't find the
      // requested path(s)
      return std::nullopt;
    }

    rootPath = next;
  }
}

std::unique_ptr<SCM> SCM::scmForPath(w_string_piece rootPath) {
  auto scmRoot = findFileInDirTree(rootPath, {kHg, kGit});
  if (!scmRoot) {
    return nullptr;
  }
  auto root = scmRoot->piece();

  auto base = root.baseName();

  if (base == kHg) {
    return std::make_unique<Mercurial>(rootPath, root.dirName());
  }

  if (base == kGit) {
    return std::make_unique<Git>(rootPath, root.dirName());
  }

  return nullptr;
}
} // namespace watchman
