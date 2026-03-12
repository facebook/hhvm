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

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;

namespace FBThrift
{
    /// <summary>
    /// Shared protocol logic used by both Binary and Compact readers/writers.
    /// Eliminates duplication of Skip, ReadValue, WriteValue, and GetThriftType
    /// across the two protocol implementations.
    /// </summary>
    public static class ThriftProtocolHelper
    {
        /// <summary>
        /// Default maximum nesting depth for skip operations.
        /// Matches Rust's DEFAULT_RECURSION_DEPTH. C++ uses 12,000 but 64 is
        /// a safer default that still supports all practical payloads.
        /// </summary>
        public const int DefaultMaxDepth = 64;

        // Cache compiled delegates for single-type generic methods (ReadListTyped, ReadHashSetTyped, WriteCollectionTyped).
        // Uses Delegate.CreateDelegate for ~100x faster invocation vs MethodInfo.Invoke, with no
        // object[] allocation or value-type boxing per call.
        private static readonly ConcurrentDictionary<Type, Func<IThriftProtocolReader, int, object>> s_readListCache = new();
        private static readonly ConcurrentDictionary<Type, Func<IThriftProtocolReader, int, object>> s_readHashSetCache = new();
        private static readonly ConcurrentDictionary<Type, Action<IThriftProtocolWriter, object>> s_writeCollectionCache = new();

        // Cache compiled delegates for two-type generic methods (ReadDictionaryTyped, WriteMapTyped)
        private static readonly ConcurrentDictionary<(Type, Type), Func<IThriftProtocolReader, int, object>> s_readDictionaryCache = new();
        private static readonly ConcurrentDictionary<(Type, Type), Action<IThriftProtocolWriter, System.Collections.IDictionary>> s_writeMapCache = new();

        // Cache MethodInfo for ReadStruct (keyed by reader type + struct type)
        private static readonly ConcurrentDictionary<(Type, Type), MethodInfo> s_readStructCache = new();

        // Cache for ICollection<T>.Count property
        private static readonly ConcurrentDictionary<Type, PropertyInfo> s_collectionCountCache = new();

        /// <summary>
        /// Skips a value of the given Thrift type by reading and discarding it.
        /// Enforces a nesting depth limit to prevent stack overflow from malicious payloads.
        /// </summary>
        /// <param name="fieldId">Optional field ID for richer error messages.</param>
        public static void Skip(IThriftProtocolReader reader, ThriftWireType fieldType, short? fieldId = null)
        {
            Skip(reader, fieldType, DefaultMaxDepth, fieldId);
        }

        /// <summary>
        /// Skips a value of the given Thrift type with an explicit depth limit.
        /// </summary>
        /// <param name="fieldId">Optional field ID for richer error messages.</param>
        public static void Skip(IThriftProtocolReader reader, ThriftWireType fieldType, int remainingDepth, short? fieldId = null)
        {
            if (remainingDepth <= 0)
            {
                var ctx = fieldId.HasValue
                    ? $" field id={fieldId.Value} (type={fieldType})"
                    : $" type={fieldType}";
                throw new ThriftProtocolException(
                    $"Maximum nesting depth ({DefaultMaxDepth}) exceeded while skipping{ctx} — possible malicious payload");
            }

            switch (fieldType)
            {
                case ThriftWireType.Bool:
                    reader.ReadBool();
                    break;
                case ThriftWireType.Byte:
                    reader.ReadByte();
                    break;
                case ThriftWireType.I16:
                    reader.ReadI16();
                    break;
                case ThriftWireType.I32:
                    reader.ReadI32();
                    break;
                case ThriftWireType.I64:
                    reader.ReadI64();
                    break;
                case ThriftWireType.Float:
                    reader.ReadFloat();
                    break;
                case ThriftWireType.Double:
                    reader.ReadDouble();
                    break;
                case ThriftWireType.String:
                    // Use ReadBinary instead of ReadString to avoid UTF-8 decoding errors.
                    // The String wire type is used for both string and binary fields, and
                    // when skipping unknown fields we don't know which it was originally.
                    // Binary data may contain invalid UTF-8 sequences that would throw.
                    reader.ReadBinary();
                    break;
                case ThriftWireType.Struct:
                    SkipStruct(reader, remainingDepth - 1);
                    break;
                case ThriftWireType.Map:
                    SkipMap(reader, remainingDepth - 1);
                    break;
                case ThriftWireType.Set:
                case ThriftWireType.List:
                    SkipListOrSet(reader, remainingDepth - 1);
                    break;
                default:
                    var fieldCtx = fieldId.HasValue ? $" (field id={fieldId.Value})" : "";
                    throw new ThriftProtocolException($"Unknown field type: {fieldType}{fieldCtx}");
            }
        }

        private static void SkipStruct(IThriftProtocolReader reader, int remainingDepth)
        {
            while (true)
            {
                var (fieldType, fieldId) = reader.ReadFieldBegin();
                if (fieldType == ThriftWireType.Stop)
                {
                    break;
                }
                Skip(reader, fieldType, remainingDepth, fieldId);
            }
        }

        private static void SkipMap(IThriftProtocolReader reader, int remainingDepth)
        {
            var (keyType, valType, size) = reader.ReadMapBegin();
            for (var i = 0; i < size; i++)
            {
                Skip(reader, keyType, remainingDepth);
                Skip(reader, valType, remainingDepth);
            }
        }

        private static void SkipListOrSet(IThriftProtocolReader reader, int remainingDepth)
        {
            var (elemType, size) = reader.ReadListBegin();
            for (var i = 0; i < size; i++)
            {
                Skip(reader, elemType, remainingDepth);
            }
        }

        /// <summary>
        /// Generic method to read any supported value type from a protocol reader.
        /// Container types use reflection to call ReadValue recursively on the reader.
        /// </summary>
        public static T ReadValue<T>(IThriftProtocolReader reader)
        {
            var type = typeof(T);

            if (type == typeof(bool))
            {
                return (T)(object)reader.ReadBool();
            }
            if (type == typeof(sbyte))
            {
                return (T)(object)reader.ReadByte();
            }
            if (type == typeof(short))
            {
                return (T)(object)reader.ReadI16();
            }
            if (type == typeof(int))
            {
                return (T)(object)reader.ReadI32();
            }
            if (type == typeof(long))
            {
                return (T)(object)reader.ReadI64();
            }
            if (type == typeof(float))
            {
                return (T)(object)reader.ReadFloat();
            }
            if (type == typeof(double))
            {
                return (T)(object)reader.ReadDouble();
            }
            if (type == typeof(string))
            {
                return (T)(object)reader.ReadString();
            }
            if (type == typeof(byte[]))
            {
                return (T)(object)reader.ReadBinary();
            }
            if (typeof(IThriftSerializable).IsAssignableFrom(type))
            {
                var readerType = reader.GetType();
                var cacheKey = (readerType, type);
                var readStructMethod = s_readStructCache.GetOrAdd(cacheKey, k =>
                    k.Item1.GetMethod(nameof(IThriftProtocolReader.ReadStruct))!.MakeGenericMethod(k.Item2));
                return (T)readStructMethod.Invoke(reader, null)!;
            }
            if (type.IsEnum)
            {
                var intVal = reader.ReadI32();
                return (T)Enum.ToObject(type, intVal);
            }
            if (type.IsGenericType)
            {
                var genericDef = type.GetGenericTypeDefinition();
                if (genericDef == typeof(List<>))
                {
                    var elemType = type.GetGenericArguments()[0];
                    var (_, size) = reader.ReadListBegin();
                    return (T)ReadListInternal(reader, elemType, size);
                }
                if (genericDef == typeof(HashSet<>))
                {
                    var elemType = type.GetGenericArguments()[0];
                    var (_, size) = reader.ReadSetBegin();
                    return (T)ReadHashSetInternal(reader, elemType, size);
                }
                if (genericDef == typeof(Dictionary<,>))
                {
                    var keyType = type.GetGenericArguments()[0];
                    var valType = type.GetGenericArguments()[1];
                    var (_, _, size) = reader.ReadMapBegin();
                    return (T)ReadDictionaryInternal(reader, keyType, valType, size);
                }
            }

            throw new NotSupportedException($"Cannot read value of type {typeof(T).Name}");
        }

        // --- Delegate wrapper methods ---
        // These have signatures matching the cached delegate types so Delegate.CreateDelegate
        // can bind them directly, avoiding MethodInfo.Invoke overhead and object[] allocations.

        private static object ReadListTypedDelegate<TElem>(IThriftProtocolReader reader, int size)
        {
            return ReadListTyped<TElem>(reader, size);
        }

        private static object ReadHashSetTypedDelegate<TElem>(IThriftProtocolReader reader, int size)
        {
            return ReadHashSetTyped<TElem>(reader, size);
        }

        private static object ReadDictionaryTypedDelegate<TKey, TVal>(IThriftProtocolReader reader, int size)
            where TKey : notnull
        {
            return ReadDictionaryTyped<TKey, TVal>(reader, size);
        }

        private static void WriteCollectionTypedDelegate<TElem>(IThriftProtocolWriter writer, object collection)
        {
            WriteCollectionTyped(writer, (IEnumerable<TElem>)collection);
        }

        // --- Typed container read helpers (compiled delegate dispatch, no reflection per call) ---

        private static object ReadListInternal(IThriftProtocolReader reader, Type elemType, int size)
        {
            var invoker = s_readListCache.GetOrAdd(elemType, t =>
            {
                var mi = typeof(ThriftProtocolHelper)
                    .GetMethod(nameof(ReadListTypedDelegate), BindingFlags.NonPublic | BindingFlags.Static)!
                    .MakeGenericMethod(t);
                return (Func<IThriftProtocolReader, int, object>)Delegate.CreateDelegate(
                    typeof(Func<IThriftProtocolReader, int, object>), mi);
            });
            return invoker(reader, size);
        }

        private static List<TElem> ReadListTyped<TElem>(IThriftProtocolReader reader, int size)
        {
            var list = new List<TElem>(size);
            for (var i = 0; i < size; i++)
            {
                list.Add(ReadValue<TElem>(reader));
            }
            return list;
        }

        private static object ReadHashSetInternal(IThriftProtocolReader reader, Type elemType, int size)
        {
            var invoker = s_readHashSetCache.GetOrAdd(elemType, t =>
            {
                var mi = typeof(ThriftProtocolHelper)
                    .GetMethod(nameof(ReadHashSetTypedDelegate), BindingFlags.NonPublic | BindingFlags.Static)!
                    .MakeGenericMethod(t);
                return (Func<IThriftProtocolReader, int, object>)Delegate.CreateDelegate(
                    typeof(Func<IThriftProtocolReader, int, object>), mi);
            });
            return invoker(reader, size);
        }

        private static HashSet<TElem> ReadHashSetTyped<TElem>(IThriftProtocolReader reader, int size)
        {
            var set = new HashSet<TElem>(size);
            for (var i = 0; i < size; i++)
            {
                set.Add(ReadValue<TElem>(reader));
            }
            return set;
        }

        private static object ReadDictionaryInternal(IThriftProtocolReader reader, Type keyType, Type valType, int size)
        {
            var cacheKey = (keyType, valType);
            var invoker = s_readDictionaryCache.GetOrAdd(cacheKey, k =>
            {
                var mi = typeof(ThriftProtocolHelper)
                    .GetMethod(nameof(ReadDictionaryTypedDelegate), BindingFlags.NonPublic | BindingFlags.Static)!
                    .MakeGenericMethod(k.Item1, k.Item2);
                return (Func<IThriftProtocolReader, int, object>)Delegate.CreateDelegate(
                    typeof(Func<IThriftProtocolReader, int, object>), mi);
            });
            return invoker(reader, size);
        }

        private static Dictionary<TKey, TVal> ReadDictionaryTyped<TKey, TVal>(IThriftProtocolReader reader, int size)
            where TKey : notnull
        {
            var dict = new Dictionary<TKey, TVal>(size);
            for (var i = 0; i < size; i++)
            {
                var key = ReadValue<TKey>(reader);
                var val = ReadValue<TVal>(reader);
                dict[key] = val;
            }
            return dict;
        }

        /// <summary>
        /// Generic method to write any supported value type to a protocol writer.
        /// </summary>
        public static void WriteValue<T>(IThriftProtocolWriter writer, T value)
        {
            switch (value)
            {
                case bool b:
                    writer.WriteBool(b);
                    break;
                case sbyte sb:
                    writer.WriteByte(sb);
                    break;
                case short s:
                    writer.WriteI16(s);
                    break;
                case int i:
                    writer.WriteI32(i);
                    break;
                case long l:
                    writer.WriteI64(l);
                    break;
                case float f:
                    writer.WriteFloat(f);
                    break;
                case double d:
                    writer.WriteDouble(d);
                    break;
                case string str:
                    writer.WriteString(str);
                    break;
                case byte[] bytes:
                    writer.WriteBinary(bytes);
                    break;
                case IThriftSerializable serializable:
                    writer.WriteStruct(serializable);
                    break;
                case Enum e:
                    writer.WriteI32(Convert.ToInt32(e));
                    break;
                case System.Collections.IDictionary dict:
                    WriteMapValue(writer, dict);
                    break;
                case System.Collections.ICollection collection:
                    WriteCollectionValue(writer, collection);
                    break;
                default:
                    var type = typeof(T);
                    if (type.IsGenericType)
                    {
                        var interfaces = type.GetInterfaces();
                        foreach (var iface in interfaces)
                        {
                            if (iface.IsGenericType)
                            {
                                var genericDef = iface.GetGenericTypeDefinition();
                                if (genericDef == typeof(ICollection<>) || genericDef == typeof(ISet<>))
                                {
                                    WriteGenericCollectionValue(writer, value);
                                    return;
                                }
                            }
                        }
                    }
                    throw new NotSupportedException($"Cannot write value of type {typeof(T).Name}");
            }
        }

        /// <summary>
        /// Returns the Thrift type ID for a given CLR type.
        /// </summary>
        public static ThriftWireType GetThriftType(Type type)
        {
            if (type == typeof(bool)) return ThriftWireType.Bool;
            if (type == typeof(sbyte)) return ThriftWireType.Byte;
            if (type == typeof(short)) return ThriftWireType.I16;
            if (type == typeof(int)) return ThriftWireType.I32;
            if (type == typeof(long)) return ThriftWireType.I64;
            if (type == typeof(float)) return ThriftWireType.Float;
            if (type == typeof(double)) return ThriftWireType.Double;
            if (type == typeof(string)) return ThriftWireType.String;
            if (type == typeof(byte[])) return ThriftWireType.String;
            if (typeof(IThriftSerializable).IsAssignableFrom(type)) return ThriftWireType.Struct;
            if (type.IsEnum) return ThriftWireType.I32;
            if (type.IsGenericType)
            {
                var genericDef = type.GetGenericTypeDefinition();
                if (genericDef == typeof(List<>)) return ThriftWireType.List;
                if (genericDef == typeof(HashSet<>)) return ThriftWireType.Set;
                if (genericDef == typeof(Dictionary<,>)) return ThriftWireType.Map;
            }
            throw new NotSupportedException($"Cannot determine Thrift type for {type.Name}");
        }

        /// <summary>
        /// Generic version of GetThriftType for use with collection helpers.
        /// </summary>
        public static ThriftWireType GetWireType<T>()
        {
            return GetThriftType(typeof(T));
        }

        private static void WriteMapValue(IThriftProtocolWriter writer, System.Collections.IDictionary dict)
        {
            var dictType = dict.GetType();
            if (!dictType.IsGenericType)
            {
                throw new NotSupportedException(
                    $"Cannot write non-generic dictionary type {dictType.Name}. Use Dictionary<TKey, TValue> instead.");
            }
            var genericArgs = dictType.GetGenericArguments();
            var keyType = genericArgs[0];
            var valType = genericArgs[1];
            writer.WriteMapBegin(GetThriftType(keyType), GetThriftType(valType), dict.Count);

            var cacheKey = (keyType, valType);
            var invoker = s_writeMapCache.GetOrAdd(cacheKey, k =>
            {
                var mi = typeof(ThriftProtocolHelper)
                    .GetMethod(nameof(WriteMapTyped), BindingFlags.NonPublic | BindingFlags.Static)!
                    .MakeGenericMethod(k.Item1, k.Item2);
                return (Action<IThriftProtocolWriter, System.Collections.IDictionary>)Delegate.CreateDelegate(
                    typeof(Action<IThriftProtocolWriter, System.Collections.IDictionary>), mi);
            });
            invoker(writer, dict);
        }

        private static void WriteMapTyped<TKey, TVal>(IThriftProtocolWriter writer, System.Collections.IDictionary dict)
        {
            foreach (var kvp in (IDictionary<TKey, TVal>)dict)
            {
                WriteValue(writer, kvp.Key);
                WriteValue(writer, kvp.Value);
            }
        }

        private static void WriteCollectionValue(IThriftProtocolWriter writer, System.Collections.ICollection collection)
        {
            var collType = collection.GetType();
            if (!collType.IsGenericType)
            {
                throw new NotSupportedException(
                    $"Cannot write non-generic collection type {collType.Name}. Use List<T> or HashSet<T> instead.");
            }
            var elemType = collType.GetGenericArguments()[0];
            var thriftType = GetThriftType(elemType);
            var isSet = collType.GetGenericTypeDefinition() == typeof(HashSet<>);
            if (isSet)
            {
                writer.WriteSetBegin(thriftType, collection.Count);
            }
            else
            {
                writer.WriteListBegin(thriftType, collection.Count);
            }

            var invoker = s_writeCollectionCache.GetOrAdd(elemType, t =>
            {
                var mi = typeof(ThriftProtocolHelper)
                    .GetMethod(nameof(WriteCollectionTypedDelegate), BindingFlags.NonPublic | BindingFlags.Static)!
                    .MakeGenericMethod(t);
                return (Action<IThriftProtocolWriter, object>)Delegate.CreateDelegate(
                    typeof(Action<IThriftProtocolWriter, object>), mi);
            });
            invoker(writer, collection);
        }

        private static void WriteCollectionTyped<TElem>(IThriftProtocolWriter writer, IEnumerable<TElem> collection)
        {
            foreach (var item in collection)
            {
                WriteValue(writer, item);
            }
        }

        private static void WriteCollectionTypedWrapper<TElem>(IThriftProtocolWriter writer, object collection)
        {
            WriteCollectionTyped(writer, (IEnumerable<TElem>)collection);
        }

        private static void WriteGenericCollectionValue<T>(IThriftProtocolWriter writer, T collection)
        {
            var collType = collection!.GetType();
            var elemType = collType.GetGenericArguments()[0];
            var thriftType = GetThriftType(elemType);

            bool isSet = false;
            var interfaces = collType.GetInterfaces();
            foreach (var iface in interfaces)
            {
                if (iface.IsGenericType)
                {
                    var genericDef = iface.GetGenericTypeDefinition();
                    if (genericDef == typeof(ISet<>))
                    {
                        isSet = true;
                        break;
                    }
                }
            }

            if (!isSet && collType.IsGenericType)
            {
                var genericDef = collType.GetGenericTypeDefinition();
                isSet = genericDef == typeof(HashSet<>);
            }

            // Get Count via ICollection<T> interface (generic collections may not implement non-generic ICollection)
            var countProp = s_collectionCountCache.GetOrAdd(elemType, t =>
                typeof(ICollection<>).MakeGenericType(t).GetProperty("Count")!);
            int count = (int)countProp.GetValue(collection)!;

            if (isSet)
            {
                writer.WriteSetBegin(thriftType, count);
            }
            else
            {
                writer.WriteListBegin(thriftType, count);
            }

            var invoker = s_writeCollectionCache.GetOrAdd(elemType, t =>
            {
                var mi = typeof(ThriftProtocolHelper)
                    .GetMethod(nameof(WriteCollectionTypedDelegate), BindingFlags.NonPublic | BindingFlags.Static)!
                    .MakeGenericMethod(t);
                return (Action<IThriftProtocolWriter, object>)Delegate.CreateDelegate(
                    typeof(Action<IThriftProtocolWriter, object>), mi);
            });
            invoker(writer, collection);
        }
    }
}
