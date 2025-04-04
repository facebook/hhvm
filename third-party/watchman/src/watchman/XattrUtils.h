/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace watchman {

/**
 * Sets extended attributes on a file to allow secondary group access. This
 * function avoids needing to use chmod which wont work if the user is not a
 * member of the secondary group.
 *
 * @param path The path to the file to set attributes on
 * @param secondary_group_name The name of the secondary group
 * @param read If the secondary group should be given read permissions on the
 * file
 * @param write If the secondary group should be given write permissions on the
 * file
 * @param execute If the secondary group should be given execute permissions on
 * the file
 * @return On Linux, true if the attributes were set successfully, false on
 * error. Always returns false on non-Linux
 */
bool setFileSecondaryGroupACL(
    const char* path,
    const char* secondary_group_name,
    bool read,
    bool write,
    bool execute);

} // namespace watchman
