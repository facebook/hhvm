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
#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/cls-cns-profile.h"
#include "hphp/runtime/vm/jit/coeffect-fun-param-profile.h"
#include "hphp/runtime/vm/jit/cow-profile.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/incref-profile.h"
#include "hphp/runtime/vm/jit/is-type-struct-profile.h"
#include "hphp/runtime/vm/jit/prof-data-target-profile.h"
#include "hphp/runtime/vm/jit/switch-profile.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type-profile.h"
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
  static_assert(std::is_trivially_copyable_v<T>);

  auto const bytes = reinterpret_cast<const uint8_t*>(&value);
  writer.payload.insert(writer.payload.end(), bytes, bytes + sizeof(value));
}

template<class T>
void read_raw(PayloadReader& reader, T& value) {
  static_assert(std::is_trivially_copyable_v<T>);

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

template<class T>
bool isValidRawTargetProfileSize(size_t size) {
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::has_unique_object_representations_v<T>);

  if constexpr (std::is_same_v<T, SwitchProfile>) {
    static_assert(sizeof(SwitchProfile) == sizeof(uint32_t));

    return size >= sizeof(SwitchProfile) && size % sizeof(uint32_t) == 0;
  } else {
    return size == sizeof(T);
  }
}

template<class T>
bool isValidRawTargetProfile(const ContProfTargetProfile& profile) {
  // The kind determines the layout; names are opaque RDS lookup keys.
  return profile.bytecodeOffset >= 0 &&
    isValidRawTargetProfileSize<T>(profile.payload.size());
}

template<class T>
std::optional<ContProfTargetProfile>
snapshotRawTargetProfile(
  const rds::Profile& profile,
  rds::Handle handle,
  uint32_t allocationSize,
  ContProfTargetProfileKind kind
) {
  if (profile.bcOff < 0) return std::nullopt;

  assertx(profile.name);
  assertx(isValidRawTargetProfileSize<T>(allocationSize));

  std::vector<uint8_t> payload(allocationSize);
  auto const captured = TargetProfile<T>::withTemporary(
    allocationSize, [&] (T& reduced) {
      TargetProfile<T>::reduce(reduced, handle, allocationSize);
      std::memcpy(payload.data(), &reduced, payload.size());
    }
  );
  if (!captured) return std::nullopt;

  return ContProfTargetProfile{
    kind,
    profile.bcOff,
    profile.name->toCppString(),
    std::move(payload),
  };
}

template<class T>
bool installRawTargetProfile(
  const ContProfPreparedTargetProfile& profile,
  TransID transId,
  ProfDataTargetProfile& targetProfiles
) {
  assertx(profile.bytecodeOffset >= 0);
  assertx(profile.name);
  assertx(isValidRawTargetProfileSize<T>(profile.payload.size()));

  auto const key = rds::Profile{
    static_cast<T*>(nullptr),
    transId,
    profile.bytecodeOffset,
    profile.name,
  };

  auto const memory = std::malloc(profile.payload.size());
  if (!memory) return false;
  std::unique_ptr<T, decltype(&std::free)> value{
    new (memory) T{}, &std::free
  };
  std::memcpy(value.get(), profile.payload.data(), profile.payload.size());
  targetProfiles.add(key, value.get());
  value.release();
  return true;
}

void writeType(PayloadWriter& writer, Type type) {
  // These profiles contain cell types, so no pointer location is needed.
  // Store only the basic bits: constants and specializations can contain
  // process-local pointers.
  write_raw(writer, type.rawBits());
}

std::optional<Type> readType(PayloadReader& reader) {
  Type::bits_t bits{};
  read_raw(reader, bits);
  if (reader.failed || (bits & Type::kCell) != bits) {
    return std::nullopt;
  }

  return Type{bits, PtrLocation::Bottom};
}

std::vector<uint8_t> encodeTypeProfilePayload(const TypeProfile& profile) {
  PayloadWriter writer;
  writer.payload.reserve(sizeof(profile));
  writeType(writer, profile.type);
  write_raw(writer, profile.count);
  return std::move(writer.payload);
}

std::optional<TypeProfile>
decodeTypeProfilePayload(const std::vector<uint8_t>& payload) {
  PayloadReader reader{payload.data(), payload.size()};
  auto const type = readType(reader);
  if (!type) return std::nullopt;

  TypeProfile result{};
  result.type = *type;
  read_raw(reader, result.count);
  if (reader.failed || reader.remaining != 0) return std::nullopt;
  return result;
}

std::vector<uint8_t>
encodeArrayIterProfilePayload(const ArrayIterProfile& profile) {
  auto const result = profile.result();

  PayloadWriter writer;
  writer.payload.reserve(sizeof(profile));
  write_raw(writer, result.key_types.toBits());
  writeType(writer, result.value_type);
  return std::move(writer.payload);
}

std::optional<ArrayIterProfile>
decodeArrayIterProfilePayload(const std::vector<uint8_t>& payload) {
  PayloadReader reader{payload.data(), payload.size()};
  uint8_t keyTypes{};
  read_raw(reader, keyTypes);
  auto const allowedKeyTypes = ArrayKeyTypes::Any().toBits();
  if ((keyTypes | allowedKeyTypes) != allowedKeyTypes) {
    return std::nullopt;
  }

  auto const valueType = readType(reader);
  if (!valueType || !(*valueType <= TInitCell)) return std::nullopt;
  if (reader.remaining != 0) return std::nullopt;

  return ArrayIterProfile{
    .m_key_types = ArrayKeyTypes::FromBits(keyTypes),
    .m_value_type = *valueType,
  };
}

std::optional<ContProfTargetProfile>
snapshotTypeTargetProfile(
  const rds::Profile& profile,
  rds::Handle handle,
  uint32_t allocationSize
) {
  if (profile.bcOff < 0) return std::nullopt;

  assertx(profile.name);
  assertx(allocationSize == sizeof(TypeProfile));

  TypeProfile reduced{};
  TargetProfile<TypeProfile>::reduce(reduced, handle, allocationSize);

  return ContProfTargetProfile{
    ContProfTargetProfileKind::Type,
    profile.bcOff,
    profile.name->toCppString(),
    encodeTypeProfilePayload(reduced),
  };
}

std::optional<ContProfTargetProfile>
snapshotArrayIterTargetProfile(
  const rds::Profile& profile,
  rds::Handle handle,
  uint32_t allocationSize
) {
  if (profile.bcOff < 0) return std::nullopt;

  assertx(profile.name);
  assertx(allocationSize == sizeof(ArrayIterProfile));

  ArrayIterProfile reduced{};
  TargetProfile<ArrayIterProfile>::reduce(reduced, handle, allocationSize);

  return ContProfTargetProfile{
    ContProfTargetProfileKind::ArrayIter,
    profile.bcOff,
    profile.name->toCppString(),
    encodeArrayIterProfilePayload(reduced),
  };
}

bool installTypeTargetProfile(
  const ContProfPreparedTargetProfile& profile,
  TransID transId,
  ProfDataTargetProfile& targetProfiles
) {
  assertx(profile.kind == ContProfTargetProfileKind::Type);
  assertx(profile.bytecodeOffset >= 0);
  assertx(profile.name);

  auto const decoded = decodeTypeProfilePayload(profile.payload);
  if (!decoded) return false;

  return addTargetProfileValue(
    rds::Profile{
      static_cast<TypeProfile*>(nullptr),
      transId,
      profile.bytecodeOffset,
      profile.name,
    },
    *decoded,
    targetProfiles
  );
}

bool installArrayIterTargetProfile(
  const ContProfPreparedTargetProfile& profile,
  TransID transId,
  ProfDataTargetProfile& targetProfiles
) {
  assertx(profile.kind == ContProfTargetProfileKind::ArrayIter);
  assertx(profile.bytecodeOffset >= 0);
  assertx(profile.name);

  auto const decoded = decodeArrayIterProfilePayload(profile.payload);
  if (!decoded) return false;

  return addTargetProfileValue(
    rds::Profile{
      static_cast<ArrayIterProfile*>(nullptr),
      transId,
      profile.bytecodeOffset,
      profile.name,
    },
    *decoded,
    targetProfiles
  );
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

    case ContProfTargetProfileKind::COW:
      return isValidRawTargetProfile<COWProfile>(profile);

    case ContProfTargetProfileKind::CoeffectFunParam:
      return isValidRawTargetProfile<CoeffectFunParamProfile>(profile);

    case ContProfTargetProfileKind::IncRef:
      return isValidRawTargetProfile<IncRefProfile>(profile);

    case ContProfTargetProfileKind::IsTypeStruct:
      return isValidRawTargetProfile<IsTypeStructProfile>(profile);

    case ContProfTargetProfileKind::Switch:
      return isValidRawTargetProfile<SwitchProfile>(profile);

    case ContProfTargetProfileKind::ArrayAccess:
      return isValidRawTargetProfile<ArrayAccessProfile>(profile);

    case ContProfTargetProfileKind::ClsCns:
      return isValidRawTargetProfile<ClsCnsProfile>(profile);

    case ContProfTargetProfileKind::Type:
      return profile.bytecodeOffset >= 0 &&
        decodeTypeProfilePayload(profile.payload).has_value();

    case ContProfTargetProfileKind::ArrayIter:
      return profile.bytecodeOffset >= 0 &&
        decodeArrayIterProfilePayload(profile.payload).has_value();
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

    case rds::ProfileKind::COWProfile:
      return snapshotRawTargetProfile<COWProfile>(
        profile,
        handle,
        allocationSize,
        ContProfTargetProfileKind::COW
      );

    case rds::ProfileKind::CoeffectFunParamProfile:
      return snapshotRawTargetProfile<CoeffectFunParamProfile>(
        profile,
        handle,
        allocationSize,
        ContProfTargetProfileKind::CoeffectFunParam
      );

    case rds::ProfileKind::IncRefProfile:
      return snapshotRawTargetProfile<IncRefProfile>(
        profile,
        handle,
        allocationSize,
        ContProfTargetProfileKind::IncRef
      );

    case rds::ProfileKind::IsTypeStructProfile:
      return snapshotRawTargetProfile<IsTypeStructProfile>(
        profile,
        handle,
        allocationSize,
        ContProfTargetProfileKind::IsTypeStruct
      );

    case rds::ProfileKind::SwitchProfile:
      return snapshotRawTargetProfile<SwitchProfile>(
        profile,
        handle,
        allocationSize,
        ContProfTargetProfileKind::Switch
      );

    case rds::ProfileKind::ArrayAccessProfile:
      return snapshotRawTargetProfile<ArrayAccessProfile>(
        profile,
        handle,
        allocationSize,
        ContProfTargetProfileKind::ArrayAccess
      );

    case rds::ProfileKind::ClsCnsProfile:
      return snapshotRawTargetProfile<ClsCnsProfile>(
        profile,
        handle,
        allocationSize,
        ContProfTargetProfileKind::ClsCns
      );

    case rds::ProfileKind::TypeProfile:
      return snapshotTypeTargetProfile(profile, handle, allocationSize);

    case rds::ProfileKind::ArrayIterProfile:
      return snapshotArrayIterTargetProfile(profile, handle, allocationSize);

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

    case ContProfTargetProfileKind::COW:
      return installRawTargetProfile<COWProfile>(
        profile,
        transId,
        targetProfiles
      );

    case ContProfTargetProfileKind::CoeffectFunParam:
      return installRawTargetProfile<CoeffectFunParamProfile>(
        profile,
        transId,
        targetProfiles
      );

    case ContProfTargetProfileKind::IncRef:
      return installRawTargetProfile<IncRefProfile>(
        profile,
        transId,
        targetProfiles
      );

    case ContProfTargetProfileKind::IsTypeStruct:
      return installRawTargetProfile<IsTypeStructProfile>(
        profile,
        transId,
        targetProfiles
      );

    case ContProfTargetProfileKind::Switch:
      return installRawTargetProfile<SwitchProfile>(
        profile,
        transId,
        targetProfiles
      );

    case ContProfTargetProfileKind::ArrayAccess:
      return installRawTargetProfile<ArrayAccessProfile>(
        profile,
        transId,
        targetProfiles
      );

    case ContProfTargetProfileKind::ClsCns:
      return installRawTargetProfile<ClsCnsProfile>(
        profile,
        transId,
        targetProfiles
      );

    case ContProfTargetProfileKind::Type:
      return installTypeTargetProfile(profile, transId, targetProfiles);

    case ContProfTargetProfileKind::ArrayIter:
      return installArrayIterTargetProfile(profile, transId, targetProfiles);
  }

  return false;
}

}
