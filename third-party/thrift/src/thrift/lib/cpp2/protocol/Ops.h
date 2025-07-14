/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/protocol/FieldMaskRef.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/detail/DynamicCursorSerializer.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>

namespace apache::thrift::protocol {
// Returns a serialized buffer containing a new object that contains only the
// masked fields present in the input serialized object (which must be a
// structured type).
// input is borrowed and will be unmodified when the function returns.
template <typename Serializer, bool Contiguous = false>
std::unique_ptr<folly::IOBuf> filterSerialized(
    MaskRef mask, std::unique_ptr<folly::IOBuf>& input);

namespace detail {
using apache::thrift::detail::ContainerDynamicCursorReader;
using apache::thrift::detail::ContainerDynamicCursorWriter;
using apache::thrift::detail::DynamicCursorSerializationWrapper;
using apache::thrift::detail::StructuredDynamicCursorReader;
using apache::thrift::detail::StructuredDynamicCursorWriter;

template <typename Serializer, bool Contiguous>
struct FilterVisitor {
  using Reader = typename Serializer::ProtocolReader;
  using Writer = typename Serializer::ProtocolWriter;

  DynamicCursorSerializationWrapper<Reader, Writer>* inWrapper_;

  std::unique_ptr<folly::IOBuf> operator()(
      MaskRef mask, std::unique_ptr<folly::IOBuf>& input) {
    if (!mask.isFieldMask()) {
      throw std::runtime_error("Top-level mask must be a FieldMask.");
    }

    DynamicCursorSerializationWrapper<Reader, Writer> inputWrapper(
        std::move(input));
    inWrapper_ = &inputWrapper;
    DynamicCursorSerializationWrapper<Reader, Writer> outputWrapper;

    auto reader = inputWrapper.beginRead();
    auto writer = outputWrapper.beginWrite();
    onStruct(mask, reader, writer);
    inputWrapper.endRead(std::move(reader));
    outputWrapper.endWrite(std::move(writer));
    input = std::move(inputWrapper).serializedData();
    return std::move(outputWrapper).serializedData();
  }

  template <
      typename ProtocolReader,
      typename ProtocolWriter,
      bool InnerContiguous>
  void onField(
      protocol::MaskRef mask,
      StructuredDynamicCursorReader<ProtocolReader, InnerContiguous>& reader,
      StructuredDynamicCursorWriter<ProtocolWriter>& writer) {
    onItem(mask, reader, reader.fieldType(), reader.fieldId(), writer);
  }

  template <typename Reader, typename Writer, typename FieldId>
  void onItem(
      protocol::MaskRef mask,
      Reader& reader,
      protocol::TType type,
      FieldId fieldId,
      Writer& writer) {
    auto withFieldInfo = [&](auto&& f) {
      if constexpr (std::is_same_v<FieldId, std::nullopt_t>) {
        return f();
      } else {
        return f(fieldId);
      }
    };

    auto passThrough = [&] {
      withFieldInfo([&](auto... maybeFieldId) {
        if (std::is_same_v<FilterVisitor::Reader, CompactProtocolReader> &&
            type == protocol::TType::T_BOOL) {
          // Bool fields are zero-width under Compact protocol so readRaw won't
          // work.
          writer.write(
              maybeFieldId..., type::bool_t{}, reader.read(type::bool_t{}));
        } else {
          writer.writeRaw(maybeFieldId..., type, *reader.readRaw());
        }
      });
    };

    if (mask.isNoneMask()) {
      reader.skip();
      return;
    }
    if (mask.isAllMask()) {
      passThrough();
      return;
    }

    if (type == protocol::TType::T_MAP) {
      if (mask.isAllMapMask()) {
        passThrough();
        return;
      }
      if (mask.isNoneMapMask()) {
        reader.skip();
        return;
      }

      withFieldInfo([&](auto... maybeFieldId) {
        // We need to determine the final container size.
        // TODO: single-pass optimization
        auto mapReader = reader.beginReadContainer();
        auto checkpoint = mapReader.checkpoint();
        uint32_t size = getMapSize(mask, mapReader);
        mapReader.restoreCheckpoint(checkpoint);

        auto mapWriter = writer.beginWriteContainer(
            maybeFieldId...,
            protocol::TType::T_MAP,
            size,
            mapReader.valueType(),
            mapReader.keyType());
        onMap(mask, mapReader, mapWriter);
        reader.endRead(std::move(mapReader));
        writer.endWrite(std::move(mapWriter));
      });
      return;
    }

    if (type != protocol::TType::T_STRUCT) {
      passThrough();
      return;
    }

    if (mask.isTypeMask()) {
      if (mask.isAllTypeMask()) {
        passThrough();
        return;
      }
      if (mask.isNoneTypeMask()) {
        reader.skip();
        return;
      }
      auto any = reader.read(type::struct_t<type::AnyStruct>{});
      if (onAny(mask, any)) {
        withFieldInfo([&](auto... maybeFieldId) {
          writer.write(maybeFieldId..., type::struct_t<type::AnyStruct>{}, any);
        });
      }
      return;
    }

    withFieldInfo([&](auto... maybeFieldId) {
      auto childReader = reader.beginReadStructured();
      auto childWriter = writer.beginWriteStructured(maybeFieldId...);
      onStruct(mask, childReader, childWriter);
      reader.endRead(std::move(childReader));
      writer.endWrite(std::move(childWriter));
    });
  }

  template <
      typename ProtocolReader,
      typename ProtocolWriter,
      bool InnerContiguous>
  void onStruct(
      protocol::MaskRef mask,
      StructuredDynamicCursorReader<ProtocolReader, InnerContiguous>& reader,
      StructuredDynamicCursorWriter<ProtocolWriter>& writer) {
    if (!mask.isFieldMask()) {
      throw std::runtime_error("Mask is not a FieldMask.");
    }

    while (reader.fieldType() != protocol::TType::T_STOP) {
      auto fieldMask = mask.get(FieldId{reader.fieldId()});
      onField(fieldMask, reader, writer);
    }
  }

  bool onAny(protocol::MaskRef mask, type::AnyStruct& any) {
    auto match = mask.get(*any.type());
    if (match.isAllMask()) {
      return true;
    } else if (match.isNoneMask()) {
      return false;
    }
    if (!match.isFieldMask()) {
      throw std::runtime_error("Invalid mask for Any.");
    }
    if (!any.protocol()->isStandard()) {
      throw std::runtime_error("Non-standard protocol not supported.");
    }
    auto protocol = any.protocol()->standard();

    auto filterAny = [&]<typename AnySerializer>() {
      DynamicCursorSerializationWrapper<
          typename AnySerializer::ProtocolReader,
          typename AnySerializer::ProtocolWriter>
          anyInWrapper(any.data()->clone());
      DynamicCursorSerializationWrapper<
          typename AnySerializer::ProtocolReader,
          typename AnySerializer::ProtocolWriter>
          anyOutWrapper(any.data()->clone());
      auto anyReader = anyInWrapper.beginRead();
      auto anyWriter = anyOutWrapper.beginWrite();
      onStruct(match, anyReader, anyWriter);
      anyInWrapper.endRead(std::move(anyReader));
      anyOutWrapper.endWrite(std::move(anyWriter));
      any.data() = std::move(*std::move(anyOutWrapper).serializedData());
    };

    switch (protocol) {
      case type::StandardProtocol::Binary:
        filterAny.template operator()<BinarySerializer>();
        return true;
      case type::StandardProtocol::Compact:
        filterAny.template operator()<CompactSerializer>();
        return true;
      default:
        throw std::runtime_error("Unsupported protocol for Any");
    }
  }

  template <typename ProtocolReader, bool InnerContiguous>
  uint32_t getMapSize(
      protocol::MaskRef mask,
      ContainerDynamicCursorReader<ProtocolReader, InnerContiguous>& reader) {
    bool isStringKey = reader.keyType() == protocol::TType::T_STRING;
    uint32_t size = 0;

    auto readIntKey = [&]() -> int64_t {
      switch (reader.keyType()) {
        case protocol::TType::T_BOOL:
          return reader.read(type::bool_t{});
        case protocol::TType::T_BYTE:
          return reader.read(type::byte_t{});
        case protocol::TType::T_I16:
          return reader.read(type::i16_t{});
        case protocol::TType::T_I32:
          return reader.read(type::i32_t{});
        case protocol::TType::T_I64:
          return reader.read(type::i64_t{});
        default:
          throw std::runtime_error("Invalid key type for map.");
      }
    };

    while (reader.remaining()) {
      if (isStringKey) {
        auto key = reader.read(type::string_t{});
        auto ret = mask.get(key);
        if (!ret.isNoneMask()) {
          size++;
        }
      } else {
        auto key = readIntKey();
        auto ret = mask.get(MapId{key});
        if (!ret.isNoneMask()) {
          size++;
        }
      }
      reader.skip(); // value
    }

    return size;
  }

  template <
      typename ProtocolReader,
      typename ProtocolWriter,
      bool InnerContiguous>
  void onMap(
      protocol::MaskRef mask,
      ContainerDynamicCursorReader<ProtocolReader, InnerContiguous>& reader,
      ContainerDynamicCursorWriter<ProtocolWriter>& writer) {
    bool isStringKey = reader.keyType() == protocol::TType::T_STRING;

    auto readIntKey = [&]() -> int64_t {
      switch (reader.keyType()) {
        case protocol::TType::T_BOOL:
          return reader.read(type::bool_t{});
        case protocol::TType::T_BYTE:
          return reader.read(type::byte_t{});
        case protocol::TType::T_I16:
          return reader.read(type::i16_t{});
        case protocol::TType::T_I32:
          return reader.read(type::i32_t{});
        case protocol::TType::T_I64:
          return reader.read(type::i64_t{});
        default:
          throw std::runtime_error("Invalid key type for map.");
      }
    };
    auto writeIntKey = [&](int64_t val) {
      switch (reader.keyType()) {
        case protocol::TType::T_BOOL:
          writer.write(type::bool_t{}, val);
          break;
        case protocol::TType::T_BYTE:
          writer.write(type::byte_t{}, folly::to_narrow(val));
          break;
        case protocol::TType::T_I16:
          writer.write(type::i16_t{}, folly::to_narrow(val));
          break;
        case protocol::TType::T_I32:
          writer.write(type::i32_t{}, folly::to_narrow(val));
          break;
        case protocol::TType::T_I64:
          writer.write(type::i64_t{}, val);
          break;
        default:
          throw std::runtime_error("Invalid key type for map.");
      }
    };

    while (reader.remaining()) {
      MaskRef match = [&] {
        if (isStringKey) {
          auto key = reader.read(type::string_t{});
          auto ret = mask.get(key);
          if (!ret.isNoneMask()) {
            writer.write(type::string_t{}, key);
          }
          return ret;
        } else {
          auto key = readIntKey();
          auto ret = mask.get(MapId{key});
          if (!ret.isNoneMask()) {
            writeIntKey(key);
          }
          return ret;
        }
      }();
      onItem(match, reader, reader.valueType(), std::nullopt, writer);
    }
  }
};
} // namespace detail

template <typename Serializer, bool Contiguous>
std::unique_ptr<folly::IOBuf> filterSerialized(
    MaskRef mask, std::unique_ptr<folly::IOBuf>& input) {
  return detail::FilterVisitor<Serializer, Contiguous>{}(mask, input);
}
} // namespace apache::thrift::protocol
