/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/vm/jit/cprof-target-profile.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/prof-data-target-profile.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/util/assertions.h"

namespace HPHP::jit::cprof {

namespace {

// Buffer-backed adapters for the profile's existing serialize/deserialize.
struct PayloadWriter {
  std::vector<uint8_t> payload;
};

struct PayloadReader {
  const uint8_t* data;
  size_t remaining;
  bool failed{false};
};

template<class T>
void write_raw(PayloadWriter& writer, const T& value) {
  static_assert(std::is_unsigned_v<T> || std::is_enum_v<T>);

  auto const bytes = reinterpret_cast<const uint8_t*>(&value);
  writer.payload.insert(writer.payload.end(), bytes, bytes + sizeof(value));
}

template<class T>
void read_raw(PayloadReader& reader, T& value) {
  static_assert(std::is_unsigned_v<T> || std::is_enum_v<T>);

  if (reader.failed || sizeof(value) > reader.remaining) {
    reader.failed = true;
    return;
  }

  std::memcpy(&value, reader.data, sizeof(value));
  reader.data += sizeof(value);
  reader.remaining -= sizeof(value);
}

template<class T>
bool addTargetProfileValue(
  const rds::Profile& key,
  const T& source,
  ProfDataTargetProfile& targetProfiles
) {
  static_assert(std::is_nothrow_copy_constructible_v<T>);
  static_assert(std::is_trivially_destructible_v<T>);

  auto const memory = std::malloc(sizeof(T));
  if (!memory) return false;
  std::unique_ptr<T, decltype(&std::free)> value{
    new (memory) T{source},
    &std::free,
  };

  targetProfiles.add(key, value.get());
  value.release();
  return true;
}

std::optional<int32_t> decodeDecRefProfileId(std::string_view name) {
  constexpr std::string_view kPrefix{"DecRefProfile-"};

  if (!name.starts_with(kPrefix)) return std::nullopt;

  auto const suffix = name.substr(kPrefix.size());

  int32_t profileId{};
  auto const [end, error] = std::from_chars(
    suffix.data(),
    suffix.data() + suffix.size(),
    profileId
  );

  if (error != std::errc{} || end != suffix.data() + suffix.size() ||
      profileId < -1) {
    return std::nullopt;
  }

  return profileId;
}

std::vector<uint8_t> encodeDecRefPayload(const DecRefProfile& profile) {
  PayloadWriter writer;
  writer.payload.reserve(sizeof(profile));
  profile.serialize(writer);
  return std::move(writer.payload);
}

std::optional<DecRefProfile> decodeDecRefPayload(
  const std::vector<uint8_t>& payload) {
  PayloadReader reader{payload.data(), payload.size()};
  DecRefProfile result{};
  result.deserialize(reader);

  if (reader.failed || reader.remaining != 0) return std::nullopt;
  return result;
}

std::optional<int32_t>
validateDecRefTargetProfile(const ContProfTargetProfile& profile) {
  if (profile.bytecodeOffset < 0) return std::nullopt;

  auto const profileId = decodeDecRefProfileId(profile.name);
  auto const value = decodeDecRefPayload(profile.payload);

  if (!profileId || !value) return std::nullopt;

  return profileId;
}

std::optional<ContProfTargetProfile>
snapshotDecRefTargetProfile(
  const rds::Profile& profile,
  rds::Handle handle,
  uint32_t allocationSize
) {
  if (profile.bcOff < 0) return std::nullopt;

  assertx(profile.name);
  assertx(allocationSize == sizeof(DecRefProfile));
  assertx(decodeDecRefProfileId(profile.name->slice()));

  DecRefProfile reduced{};
  TargetProfile<DecRefProfile>::reduce(reduced, handle, allocationSize);
  if (reduced.total == 0) return std::nullopt;

  assertx(
    reduced.datatype == kNoDataTypesSeen ||
    reduced.datatype == kMultipleDataTypesSeen ||
    (isRealType(reduced.datatype) &&
     reduced.datatype == dt_modulo_persistence(reduced.datatype))
  );

  return ContProfTargetProfile{
    ContProfTargetProfileKind::DecRef,
    static_cast<int32_t>(profile.bcOff),
    profile.name->toCppString(),
    encodeDecRefPayload(reduced),
  };
}

bool installDecRefTargetProfile(
  const ContProfPreparedTargetProfile& profile,
  TransID transId,
  ProfDataTargetProfile& targetProfiles
) {
  assertx(profile.kind == ContProfTargetProfileKind::DecRef);
  assertx(profile.bytecodeOffset >= 0);
  assertx(profile.name);

  auto const decoded = decodeDecRefPayload(profile.payload);
  if (!decoded) return false;

  return addTargetProfileValue(
    rds::Profile{
      static_cast<DecRefProfile*>(nullptr),
      transId,
      profile.bytecodeOffset,
      profile.name,
    },
    *decoded,
    targetProfiles
  );
}

}

bool isValidContProfTargetProfile(const ContProfTargetProfile& profile) {
  switch (profile.kind) {
    case ContProfTargetProfileKind::DecRef:
      return validateDecRefTargetProfile(profile).has_value();
  }

  return false;
}

std::optional<ContProfTargetProfile>
snapshotContProfTargetProfile(
  const rds::Profile& profile,
  rds::Handle handle,
  uint32_t allocationSize
) {
  switch (profile.kind) {
    case rds::ProfileKind::DecRefProfile:
      return snapshotDecRefTargetProfile(profile, handle, allocationSize);

    default:
      return std::nullopt;
  }
}

std::optional<ContProfPreparedTargetProfile>
prepareContProfTargetProfile(const ContProfTargetProfile& profile) {
  if (!isValidContProfTargetProfile(profile)) return std::nullopt;

  return ContProfPreparedTargetProfile{
    profile.kind,
    profile.bytecodeOffset,
    makeStaticString(profile.name),
    profile.payload,
  };
}

bool installContProfTargetProfile(
  const ContProfPreparedTargetProfile& profile,
  TransID transId,
  ProfDataTargetProfile& targetProfiles
) {
  switch (profile.kind) {
    case ContProfTargetProfileKind::DecRef:
      return installDecRefTargetProfile(profile, transId, targetProfiles);
  }

  return false;
}

}
