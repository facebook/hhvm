/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

// clang-format off
#pragma once

#include <cstdint>
#include <string>

#include <proxygen/lib/utils/Export.h>

namespace proxygen {

/**
 * Codes (hashes)
 */
enum %%name_enum%% : uint8_t {
  // Code reserved to indicate the absence of a valid field.
  %%enum_prefix%%_NONE = 0,
  // Code reserved to indicate a value for which there is no perfect mapping to
  // a unique field (i.e. the default field for all other values).
  %%enum_prefix%%_OTHER = 1,

  /* The following is a placeholder for the build script to generate a list
   * of enum values.
   * Note am currently unable to prevent this comment from being present in the
   * output file.
   */
%%%%%

};

const uint8_t %%name_enum%%CommonOffset = 2;

enum class %%table_type_name%%: uint8_t {
  TABLE_CAMELCASE = 0,
  TABLE_LOWERCASE = 1,
};

class %%name%% {
 public:
  // Perfect hash function to match specified names
  FB_EXPORT static %%name_enum%% hash(const char* name, size_t len);

  FB_EXPORT inline static %%name_enum%% hash(const std::string& name) {
    return hash(name.data(), name.length());
  }

  FB_EXPORT static std::string* initNames(%%table_type_name%% type);

  /* The following is a placeholder for the build script to generate a field
   * that stores the max number of defined enum fields.
   i.e. constexpr static uint64_t num_codes;
   * Note am currently unable to prevent this comment from being present in the
   * output file.
   */
$$$$$

  static const std::string* getPointerToTable(
    %%table_type_name%% type);

  inline static const std::string* getPointerToName(%%name_enum%% code,
      %%table_type_name%% type = %%table_type_name%%::TABLE_CAMELCASE) {
    return getPointerToTable(type) + code;
  }

  inline static bool isNameFromTable(const std::string* headerName,
      %%table_type_name%% type) {
    return getCodeFromTableName(headerName, type) >=
      %%name_enum%%CommonOffset;
  }

  // This method supplements hash().  If dealing with string pointers, some
  // pointing to entries in the the name table and some not, this
  // method can be used in place of hash to reverse map a string from the
  // name table to its code.
  inline static %%name_enum%% getCodeFromTableName(
      const std::string* headerName, %%table_type_name%% type) {
    if (headerName == nullptr) {
      return %%enum_prefix%%_NONE;
    } else {
      auto diff = headerName - getPointerToTable(type);
      if (diff >= %%name_enum%%CommonOffset && diff < (long)num_codes) {
        return static_cast<%%name_enum%%>(diff);
      } else {
        return %%enum_prefix%%_OTHER;
      }
    }
  }

};

} // proxygen
// clang-format on
