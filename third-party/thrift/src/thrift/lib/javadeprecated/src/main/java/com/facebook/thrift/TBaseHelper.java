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

package com.facebook.thrift;

import com.facebook.thrift.protocol.TField;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;

public final class TBaseHelper {

  private static Class[] setFieldValueArgs = new Class[] {int.class, Object.class};

  private TBaseHelper() {}

  public static <T extends TBase & Comparable<T>> int compareTo(T a, T b) {
    if (a == null && b == null) {
      return 0;
    } else if (a == null) {
      return -1;
    } else if (b == null) {
      return +1;
    }

    if (a == b) { // performance, not correctness
      return 0;
    }

    return a.compareTo(b);
  }

  public static <T extends TEnum> int compareTo(T a, T b) {
    if (a == null && b == null) {
      return 0;
    } else if (a == null) {
      return -1;
    } else if (b == null) {
      return +1;
    }

    return compareTo(a.getValue(), b.getValue());
  }

  public static int compareTo(boolean a, boolean b) {
    return a == b ? 0 : (a ? +1 : -1); // Boolean.compareTo
  }

  public static int compareTo(byte a, byte b) {
    if (a < b) {
      return -1;
    } else if (b < a) {
      return 1;
    } else {
      return 0;
    }
  }

  public static int compareTo(short a, short b) {
    if (a < b) {
      return -1;
    } else if (b < a) {
      return 1;
    } else {
      return 0;
    }
  }

  public static int compareTo(int a, int b) {
    if (a < b) {
      return -1;
    } else if (b < a) {
      return 1;
    } else {
      return 0;
    }
  }

  public static int compareTo(long a, long b) {
    if (a < b) {
      return -1;
    } else if (b < a) {
      return 1;
    } else {
      return 0;
    }
  }

  public static int compareTo(double a, double b) {
    if (a < b) {
      return -1;
    } else if (b < a) {
      return 1;
    } else {
      return 0;
    }
  }

  public static int compareTo(float a, float b) {
    if (a < b) {
      return -1;
    } else if (b < a) {
      return 1;
    } else {
      return 0;
    }
  }

  public static int compareTo(String a, String b) {
    if (a == null && b == null) {
      return 0;
    } else if (a == null) {
      return -1;
    } else if (b == null) {
      return +1;
    }

    if (a == b) { // performance, not correctness
      return 0;
    }

    return a.compareTo(b);
  }

  public static int compareTo(byte[] a, byte[] b) {
    if (a == null && b == null) {
      return 0;
    } else if (a == null) {
      return -1;
    } else if (b == null) {
      return +1;
    }

    if (a == b) { // performance, not correctness
      return 0;
    }

    int sizeCompare = compareTo(a.length, b.length);
    if (sizeCompare != 0) {
      return sizeCompare;
    }

    for (int i = 0; i < a.length; i++) {
      int byteCompare = compareTo(a[i], b[i]);
      if (byteCompare != 0) {
        return byteCompare;
      }
    }

    return 0;
  }

  public static <T> int compareTo(List<T> a, List<T> b) {
    if (a == null && b == null) {
      return 0;
    } else if (a == null) {
      return -1;
    } else if (b == null) {
      return +1;
    }

    if (a == b) { // performance, not correctness
      return 0;
    }

    int lastComparison = compareTo(a.size(), b.size());
    if (lastComparison != 0) {
      return lastComparison;
    }

    Comparator<T> comparator = NestedStructureComparator.get();

    Iterator<T> iterA = a.iterator();
    Iterator<T> iterB = b.iterator();

    while (iterA.hasNext() && iterB.hasNext()) {
      lastComparison = comparator.compare(iterA.next(), iterB.next());
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    assert !iterA.hasNext() && !iterB.hasNext();

    return 0;
  }

  public static <T> int compareTo(Set<T> a, Set<T> b) {
    if (a == null && b == null) {
      return 0;
    } else if (a == null) {
      return -1;
    } else if (b == null) {
      return +1;
    }

    if (a == b) { // performance, not correctness
      return 0;
    }

    int lastComparison = compareTo(a.size(), b.size());
    if (lastComparison != 0) {
      return lastComparison;
    }

    Comparator<T> comparator = NestedStructureComparator.get();

    SortedSet<T> sortedA = new TreeSet<T>(comparator);
    sortedA.addAll(a);
    SortedSet<T> sortedB = new TreeSet<T>(comparator);
    sortedB.addAll(b);

    Iterator<T> iterA = sortedA.iterator();
    Iterator<T> iterB = sortedB.iterator();

    // Compare each item.
    while (iterA.hasNext() && iterB.hasNext()) {
      lastComparison = comparator.compare(iterA.next(), iterB.next());
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    assert !iterA.hasNext() && !iterB.hasNext();

    return 0;
  }

  public static <K, V> int compareTo(Map<K, V> a, Map<K, V> b) {
    int lastComparison = compareTo(a.size(), b.size());
    if (lastComparison != 0) {
      return lastComparison;
    }

    if (a == b) { // performance, not correctness
      return 0;
    }

    Comparator<K> comparatorK = NestedStructureComparator.get();
    Comparator<V> comparatorV = NestedStructureComparator.get();

    // Sort a and b so we can compare them.
    SortedMap<K, V> sortedA = new TreeMap<K, V>(comparatorK);
    sortedA.putAll(a);
    SortedMap<K, V> sortedB = new TreeMap<K, V>(comparatorK);
    sortedB.putAll(b);

    Iterator<Map.Entry<K, V>> iterA = sortedA.entrySet().iterator();
    Iterator<Map.Entry<K, V>> iterB = sortedB.entrySet().iterator();

    // Compare each item.
    while (iterA.hasNext() && iterB.hasNext()) {
      Map.Entry<K, V> entryA = iterA.next();
      Map.Entry<K, V> entryB = iterB.next();
      lastComparison = comparatorK.compare(entryA.getKey(), entryB.getKey());
      if (lastComparison != 0) {
        return lastComparison;
      }
      lastComparison = comparatorV.compare(entryA.getValue(), entryB.getValue());
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    assert !iterA.hasNext() && !iterB.hasNext();

    return 0;
  }

  /*package*/
  static int compareToUnchecked(Object a, Object b) {
    return NestedStructureComparator.get().compare(a, b);
  }

  /**
   * Comparator to compare items inside containers (lists, sets, maps), while respecting Thrift's
   * equality semantics. The type of this comparator is <code>Comparator&lt;T&gt;</code>, but it can
   * only be used with types <code>T</code> supported by Thrift: the base types, Lists, Sets, and
   * Maps of Thrift-supported types, and structs and unions.
   */
  private static class NestedStructureComparator<T> implements Comparator<T> {

    @SuppressWarnings({"unchecked", "rawtypes"})
    @Override
    public int compare(T oA, T oB) {
      if (oA == oB) { // performance, not correctness
        return 0;
      }
      if (oA == null && oB == null) {
        return 0;
      } else if (oA == null) {
        return -1;
      } else if (oB == null) {
        return 1;
      } else if (oA instanceof List<?> && oB instanceof List<?>) {
        return compareTo((List) oA, (List) oB);
      } else if (oA instanceof Set<?> && oB instanceof Set<?>) {
        return compareTo((Set) oA, (Set) oB);
      } else if (oA instanceof Map<?, ?> && oB instanceof Map<?, ?>) {
        return compareTo((Map) oA, (Map) oB);
      } else if (oA instanceof byte[]) {
        return compareTo((byte[]) oA, (byte[]) oB);
      } else if (oA.getClass() == oB.getClass()
          && (oA instanceof String
              || oA instanceof Boolean
              || oA instanceof Byte
              || oA instanceof Short
              || oA instanceof Integer
              || oA instanceof Long
              || oA instanceof Float
              || oA instanceof Double
              || oA instanceof TBase)) {
        // All the primitives are Comparable. Assume that any TBase
        // we're called on is Comparable, because otherwise the codegen
        // wouldn't have made code that calls us on it. At this point,
        // we know that oA could be typed `Comparable<? super oA.getClass()>`
        // which is to say its `compareTo` accepts any subtype of some
        // unspecified supertype of oA.getClass(), so it certainly accepts
        // things of oA.getClass() and so it accepts oB.
        return ((Comparable) oA).compareTo(oB);
      }
      throw new IllegalArgumentException("cannot compare " + oA + " and " + oB);
    }

    // All instances of NestedStructureComparator are operationally equivalent,
    // so we only need to make one.
    private static final NestedStructureComparator<?> INSTANCE =
        new NestedStructureComparator<Void>();

    @SuppressWarnings("unchecked")
    public static <T> NestedStructureComparator<T> get() {
      return (NestedStructureComparator<T>) INSTANCE;
    }
  }

  public static void toString(ByteBuffer bb, StringBuilder sb) {
    byte[] buf = bb.array();

    int arrayOffset = bb.arrayOffset();
    int origLimit = bb.limit();
    int limit = (origLimit - arrayOffset > 128) ? arrayOffset + 128 : origLimit;

    for (int i = arrayOffset; i < limit; i++) {
      if (i > arrayOffset) {
        sb.append(" ");
      }
      sb.append(paddedByteString(buf[i]));
    }
    if (origLimit != limit) {
      sb.append("...");
    }
  }

  public static String paddedByteString(byte b) {
    int extended = (b | 0x100) & 0x1ff;
    return Integer.toHexString(extended).toUpperCase().substring(1);
  }

  public static byte[] byteBufferToByteArray(ByteBuffer byteBuffer) {
    if (wrapsFullArray(byteBuffer)) {
      return byteBuffer.array();
    }
    byte[] target = new byte[byteBuffer.remaining()];
    byteBufferToByteArray(byteBuffer, target, 0);
    return target;
  }

  public static boolean wrapsFullArray(ByteBuffer byteBuffer) {
    return byteBuffer.hasArray()
        && byteBuffer.position() == 0
        && byteBuffer.arrayOffset() == 0
        && byteBuffer.remaining() == byteBuffer.capacity();
  }

  public static int byteBufferToByteArray(ByteBuffer byteBuffer, byte[] target, int offset) {
    int remaining = byteBuffer.remaining();
    System.arraycopy(
        byteBuffer.array(),
        byteBuffer.arrayOffset() + byteBuffer.position(),
        target,
        offset,
        remaining);
    return remaining;
  }

  public static ByteBuffer rightSize(ByteBuffer in) {
    if (in == null) {
      return null;
    }
    if (wrapsFullArray(in)) {
      return in;
    }
    return ByteBuffer.wrap(byteBufferToByteArray(in));
  }

  public static byte[] deepCopy(byte[] orig) {
    if (orig == null) {
      return null;
    }
    return Arrays.copyOf(orig, orig.length);
  }

  public static String deepCopy(String orig) {
    return orig; // immutable
  }

  public static boolean deepCopy(boolean orig) {
    return orig; // primitive
  }

  public static Boolean deepCopy(Boolean orig) {
    return orig; // immutable
  }

  public static byte deepCopy(byte orig) {
    return orig; // primitive
  }

  public static Byte deepCopy(Byte orig) {
    return orig; // immutable
  }

  public static short deepCopy(short orig) {
    return orig; // primitive
  }

  public static Short deepCopy(Short orig) {
    return orig; // immutable
  }

  public static int deepCopy(int orig) {
    return orig; // primitive
  }

  public static Integer deepCopy(Integer orig) {
    return orig; // immutable
  }

  public static long deepCopy(long orig) {
    return orig; // primitive
  }

  public static Long deepCopy(Long orig) {
    return orig; // immutable
  }

  public static double deepCopy(double orig) {
    return orig; // primitive
  }

  public static Double deepCopy(Double orig) {
    return orig; // primitive
  }

  public static float deepCopy(float orig) {
    return orig; // primitive
  }

  public static Float deepCopy(Float orig) {
    return orig; // immutable
  }

  public static <K, V> Map<K, V> deepCopy(Map<K, V> orig) {
    if (orig == null) {
      return null;
    }
    Map<K, V> copy = new HashMap<K, V>(orig);
    for (Map.Entry<K, V> entry : orig.entrySet()) {
      copy.put(deepCopyUnchecked(entry.getKey()), deepCopyUnchecked(entry.getValue()));
    }
    return copy;
  }

  public static <T> List<T> deepCopy(List<T> orig) {
    if (orig == null) {
      return null;
    }
    List<T> copy = new ArrayList<T>();
    for (T item : orig) {
      copy.add(deepCopyUnchecked(item));
    }
    return copy;
  }

  public static <T> Set<T> deepCopy(Set<T> orig) {
    if (orig == null) {
      return null;
    }
    Set<T> copy = new HashSet<T>();
    for (T item : orig) {
      copy.add(deepCopyUnchecked(item));
    }
    return copy;
  }

  @SuppressWarnings({"unchecked", "rawtypes"})
  public static <T extends TBase> T deepCopy(T orig) {
    return (T) orig.deepCopy();
  }

  @SuppressWarnings({"unchecked", "rawtypes"})
  public static <T extends TEnum> T deepCopy(T orig) {
    return orig;
  }

  // Called above and from TUnion
  @SuppressWarnings({"unchecked", "rawtypes"})
  /*package*/ static <T> T deepCopyUnchecked(T orig) {
    if (orig == null) {
      return null;
    }
    if (orig instanceof byte[]) {
      return (T) deepCopy((byte[]) orig);
    }
    if (orig instanceof String
        || orig instanceof Boolean
        || orig instanceof Byte
        || orig instanceof Short
        || orig instanceof Integer
        || orig instanceof Long
        || orig instanceof Double
        || orig instanceof Float) {
      return orig; // immutable
    }
    if (orig instanceof Map<?, ?>) {
      return (T) deepCopy((Map<?, ?>) orig);
    }
    if (orig instanceof List<?>) {
      return (T) deepCopy((List<?>) orig);
    }
    if (orig instanceof Set<?>) {
      return (T) deepCopy((Set<?>) orig);
    }
    if (orig instanceof TBase) {
      return (T) deepCopy((TBase) orig);
    }
    if (orig instanceof TEnum) {
      return (T) deepCopy((TEnum) orig);
    }
    throw new UnsupportedOperationException(
        "Don't know how to deepCopy something of type "
            + orig.getClass()
            + " which is weird because nothing should be calling me on something"
            + " I don't understand");
  }

  /* ********************************************************
   * Equality.
   *
   * Thrift equality is value-based, not identity-based. For
   * structs and unions, we override .equals() appropriately.
   * The contract for Lists, Maps, and Sets is that their
   * equals() method is also logical equality (see the Javadocs),
   * where elements are compared with equals(). Strings and
   * boxed primitives also have equals() which do what we want.
   * Unfortunately, byte[].equals() does reference comparison.
   * That means that, we can only rely on equals() to behave the
   * way we want on structs, unions, primitives, strings, and
   * Java containers containing only things where equals() behaves
   * properly. If a Java container contains a byte[] without
   * a struct wrapping it (a "naked binary"), we can't Java's
   * equals() to do the comparison.
   *
   * ********************************************************
   */

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(byte[] a, byte[] b) {
    return Arrays.equals(a, b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, byte[] a, byte[] b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }

    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(String a, String b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, String a, String b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /** Checks two items for logical equality. */
  public static boolean equalsNobinary(boolean a, boolean b) {
    return a == b;
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, boolean a, boolean b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(Boolean a, Boolean b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, Boolean a, Boolean b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /** Checks two items for logical equality. */
  public static boolean equalsNobinary(byte a, byte b) {
    return a == b;
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, byte a, byte b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(Byte a, Byte b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, Byte a, Byte b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /** Checks two items for logical equality. */
  public static boolean equalsNobinary(short a, short b) {
    return a == b;
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, short a, short b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(Short a, Short b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, Short a, Short b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /** Checks two items for logical equality. */
  public static boolean equalsNobinary(int a, int b) {
    return a == b;
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, int a, int b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(Integer a, Integer b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, Integer a, Integer b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /** Checks two items for logical equality. */
  public static boolean equalsNobinary(long a, long b) {
    return a == b;
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, long a, long b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(Long a, Long b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, Long a, Long b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /** Checks two items for logical equality. */
  public static boolean equalsNobinary(double a, double b) {
    return a == b;
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, double a, double b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(Double a, Double b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, Double a, Double b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /** Checks two items for logical equality. */
  public static boolean equalsNobinary(float a, float b) {
    return a == b;
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, float a, float b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsNobinary(Float a, Float b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static boolean equalsNobinary(boolean isSetA, boolean isSetB, Float a, Float b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality. The items should not contain "naked binaries"; that is,
   * neither keys nor values can be <code>byte[]</code> or containers which themselves contain naked
   * binaries. The key type and value type should both be expressible in thrift.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <K, V> boolean equalsNobinary(Map<K, V> as, Map<K, V> bs) {
    if (as == null) {
      return bs == null;
    }
    return as.equals(bs);
  }

  public static <K, V> boolean equalsNobinary(
      boolean isSetA, boolean isSetB, Map<K, V> as, Map<K, V> bs) {
    if (as == bs) { // performance, not correctness
      return true;
    }
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(as, bs);
  }

  /**
   * Checks two items for logical equality. The items should not contain "naked binaries"; that is,
   * elements can not be <code>byte[]</code> or containers which themselves contain naked binaries.
   * The element type should be expressible in thrift.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <T> boolean equalsNobinary(List<T> as, List<T> bs) {
    if (as == null) {
      return false;
    }
    return as.equals(bs);
  }

  public static <T> boolean equalsNobinary(boolean isSetA, boolean isSetB, List<T> as, List<T> bs) {
    if (as == bs) { // performance, not correctness
      return true;
    }
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(as, bs);
  }

  /**
   * Checks two items for logical equality. The items should not contain "naked binaries"; that is,
   * elements can not be <code>byte[]</code> or containers which themselves contain naked binaries.
   * The element type should be expressible in thrift.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <T> boolean equalsNobinary(Set<T> as, Set<T> bs) {
    if (as == null) {
      return bs == null;
    }
    return as.equals(bs);
  }

  public static <T> boolean equalsNobinary(boolean isSetA, boolean isSetB, Set<T> as, Set<T> bs) {
    if (as == bs) { // performance, not correctness
      return true;
    }
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(as, bs);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <T extends TBase> boolean equalsNobinary(T a, T b) {
    if (a == null) {
      return b == null;
    }
    return a.equals(b);
  }

  public static <T extends TBase> boolean equalsNobinary(boolean isSetA, boolean isSetB, T a, T b) {
    if (a == b) { // performance, not correctness
      return true;
    }
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two enum for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <T extends TEnum> boolean equalsNobinary(T a, T b) {
    // Note that `a` and `b` are @Nullable here
    if (a == null) {
      return b == null;
    }
    return a.getValue() == b.getValue();
  }

  public static <T extends TEnum> boolean equalsNobinary(boolean isSetA, boolean isSetB, T a, T b) {
    if (a == b) { // performance, not correctness
      return true;
    }
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static boolean equalsSlow(byte[] a, byte[] b) {
    if (a == null) {
      return b == null;
    }
    return Arrays.equals(a, b);
  }

  public static boolean equalsSlow(boolean isSetA, boolean isSetB, byte[] a, byte[] b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  public static <K, V> boolean equalsSlow(
      boolean isSetA, boolean isSetB, Map<K, V> a, Map<K, V> b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <K, V> boolean equalsSlow(Map<K, V> a, Map<K, V> b) {
    if (a == b) { // performance, not correctness
      return true;
    }
    if (a == null) {
      return false;
    }
    if (a.size() != b.size()) {
      return false;
    }

    @SuppressWarnings("unchecked")
    Map.Entry<K, V>[] as = a.entrySet().toArray(new Map.Entry[a.size()]);
    int firstValidA = 0;

    for (Map.Entry<K, V> elemB : b.entrySet()) {
      boolean found = false;
      for (int i = firstValidA; i < as.length; i++) {
        if (equalsSlowUnchecked(as[i].getKey(), elemB.getKey())
            && equalsSlowUnchecked(as[i].getValue(), elemB.getValue())) {
          found = true;
          as[i] = as[firstValidA];
          firstValidA++;
          break;
        }
      }
      if (!found) {
        return false;
      }
    }

    assert firstValidA == as.length;

    return true;
  }

  public static <T> boolean equalsSlow(boolean isSetA, boolean isSetB, List<T> a, List<T> b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <T> boolean equalsSlow(List<T> a, List<T> b) {
    if (a == b) { // performance, not correctness
      return true;
    }
    if (a == null) {
      return false;
    }
    if (a.size() != b.size()) {
      return false;
    }

    Iterator<T> iterA = a.iterator();
    Iterator<T> iterB = b.iterator();

    while (iterA.hasNext() && iterB.hasNext()) {
      if (!equalsNobinaryUnchecked(iterA.next(), iterB.next())) {
        return false;
      }
    }
    assert !iterA.hasNext() && !iterB.hasNext();

    return true;
  }

  public static <T> boolean equalsSlow(boolean isSetA, boolean isSetB, Set<T> a, Set<T> b) {
    if (isSetA ^ isSetB) {
      return false;
    }
    if (!isSetA) {
      return true;
    }
    return equalsNobinary(a, b);
  }

  /**
   * Checks two items for logical equality.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  public static <T> boolean equalsSlow(Set<T> a, Set<T> b) {
    if (a == b) { // performance, not correctness
      return true;
    }
    if (a == null) {
      return false;
    }
    if (a.size() != b.size()) {
      return false;
    }

    @SuppressWarnings("unchecked")
    T[] as = a.toArray((T[]) new Object[a.size()]);
    int firstValidA = 0;

    for (T elemB : b) {
      boolean found = false;
      for (int i = firstValidA; i < as.length; i++) {
        if (equalsSlowUnchecked(as[i], elemB)) {
          found = true;
          as[i] = as[firstValidA];
          firstValidA++;
          break;
        }
      }
      if (!found) {
        return false;
      }
    }

    assert firstValidA == as.length;

    return true;
  }

  /**
   * Checks two items for logical equality. Neither item should contain naked binaries. Both items
   * should be of the same Thrift-expressible type.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  @SuppressWarnings({"unchecked", "rawtypes"})
  /*package*/ static <T> boolean equalsNobinaryUnchecked(T a, T b) {
    if (a == b) { // performance, not correctness
      return true;
    }
    if (a == null) {
      return false;
    }
    if (a instanceof byte[] && b instanceof byte[]) {
      return equalsSlow((byte[]) a, (byte[]) b);
    } else if (a.getClass() == b.getClass()
        && (a instanceof String
            || a instanceof Boolean
            || a instanceof Byte
            || a instanceof Short
            || a instanceof Integer
            || a instanceof Long
            || a instanceof Double
            || a instanceof Float)) {
      return a.equals(b);
    } else if (a instanceof TBase) {
      return a.equals(b);
    } else if (a instanceof TEnum) {
      return a.equals(b);
    } else if (a instanceof Map && b instanceof Map) {
      return equalsNobinary((Map) a, (Map) b);
    } else if (a instanceof List && b instanceof List) {
      return equalsNobinary((List) a, (List) b);
    } else if (a instanceof Set && b instanceof Set) {
      return equalsNobinary((Set) a, (Set) b);
    } else {
      throw new IllegalAccessError(
          "Don't know how to compare "
              + a
              + " and "
              + b
              + " which is odd, because nothing should be calling me on types"
              + " I don't understand");
    }
  }

  /**
   * Checks two items for logical equality. Both items should be of the same Thrift-expressible
   * type.
   *
   * @param a @Nullable item
   * @param b @Nullable item
   */
  @SuppressWarnings({"rawtypes", "unchecked"})
  /*package*/ static <T> boolean equalsSlowUnchecked(T a, T b) {
    if (a == b) { // performance, not correctness
      return true;
    }
    if (a == null) {
      return false;
    }
    if (a instanceof byte[] && b instanceof byte[]) {
      return equalsSlow((byte[]) a, (byte[]) b);
    } else if (a.getClass() == b.getClass()
        && (a instanceof String
            || a instanceof Boolean
            || a instanceof Byte
            || a instanceof Short
            || a instanceof Integer
            || a instanceof Long
            || a instanceof Double
            || a instanceof Float)) {
      return a.equals(b);
    } else if (a instanceof TBase) {
      return a.equals(b);
    } else if (a instanceof TEnum) {
      return ((TEnum) a).getValue() == ((TEnum) b).getValue();
    } else if (a instanceof Map && b instanceof Map) {
      return equalsSlow((Map) a, (Map) b);
    } else if (a instanceof List && b instanceof List) {
      return equalsSlow((List) a, (List) b);
    } else if (a instanceof Set && b instanceof Set) {
      return equalsSlow((Set) a, (Set) b);
    } else {
      throw new IllegalAccessError(
          "Don't know how to compare "
              + a
              + " and "
              + b
              + " which is odd, because nothing should be calling me on types"
              + " I don't understand");
    }
  }

  public static String getIndentedString(int indent) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < indent; i++) {
      sb.append("  ");
    }
    return sb.toString();
  }

  public static String reduceIndent(String indentStr) {
    return (indentStr.length() > 1) ? indentStr.substring(0, indentStr.length() - 2) : "";
  }

  /**
   * Creates String representation of object with desired indentation
   *
   * @param obj The object to be printed
   * @param indent The level of indentation
   */
  public static String toString(Object obj, int indent, boolean prettyPrint) {
    if (obj == null) {
      return "null";
    }
    String indentStr = prettyPrint ? getIndentedString(indent) : "";
    String newLine = prettyPrint ? "\n" : "";
    String space = prettyPrint ? " " : "";
    StringBuilder sb = new StringBuilder();
    try {
      if (obj instanceof Map) {
        sb.append("{");
        for (Map.Entry<?, ?> entry : (((Map<?, ?>) obj).entrySet())) {
          Object key = (Object) entry.getKey();
          Object val = (Object) entry.getValue();
          sb.append(
              newLine
                  + indentStr
                  + toString(key, indent + 2, prettyPrint)
                  + space
                  + ":"
                  + space
                  + toString(val, indent + 2, prettyPrint));
        }
        sb.append(newLine + reduceIndent(indentStr) + "}");
      } else if (obj instanceof java.util.Collection) {
        sb.append("[");
        for (Object o : (java.util.Collection<?>) obj) {
          sb.append(newLine + indentStr + toString(o, indent + 2, prettyPrint));
        }
        sb.append(newLine + reduceIndent(indentStr) + "]");
      } else if (obj instanceof TBase) {
        sb.append(((TBase) obj).toString(indent, prettyPrint));
      } else if (obj instanceof String) {
        if (prettyPrint) {
          sb.append("\"" + obj + "\"");
        } else {
          sb.append(obj);
        }
      } else {
        sb.append(obj.toString());
      }
    } catch (RuntimeException re) {
      sb.append("Exception occured :" + re.getClass() + re.getMessage());
    }
    return sb.toString();
  }

  public static String toStringHelper(TBase struct, int indent, boolean prettyPrint) {
    return toStringHelper(new StringBuilder(), struct, indent, prettyPrint);
  }

  private static String toStringHelper(
      StringBuilder sb, TBase struct, int indent, boolean prettyPrint) {
    if (struct == null) {
      return "null";
    }
    String indentStr = prettyPrint ? getIndentedString(indent) : "";
    String newLine = prettyPrint ? "\n" : "";
    String space = prettyPrint ? " " : "";
    sb.append(struct.getClass().getSimpleName());
    sb.append(space);
    sb.append("(");
    sb.append(newLine);

    try {
      boolean first = true;
      for (Field field : struct.getClass().getDeclaredFields()) {
        if (field.getType() != TField.class) {
          continue;
        }
        field.setAccessible(true);
        TField tfield = (TField) field.get(struct);
        Field f = struct.getClass().getDeclaredField(tfield.name);
        Object fieldValue = f.get(struct);

        if (!first) {
          sb.append("," + newLine);
        }
        first = false;
        sb.append(indentStr);
        sb.append(tfield.name);
        sb.append(space);
        sb.append(":");
        sb.append(space);

        if (tfield.metadata.containsKey("sensitive")) {
          sb.append("<SENSITIVE FIELD>");
        } else {
          renderFieldValue(sb, fieldValue, indent, prettyPrint, indentStr, newLine, space);
        }
      }
      sb.append(newLine + TBaseHelper.reduceIndent(indentStr));
      sb.append(")");
    } catch (Exception e) {
      sb.append("Exception occured :" + e.getClass() + " " + e.getMessage());
    }

    return sb.toString();
  }

  private static void renderFieldValue(
      StringBuilder sb,
      Object fieldValue,
      int indent,
      boolean prettyPrint,
      String indentStr,
      String newLine,
      String space) {
    if (fieldValue == null) {
      sb.append("null");
    } else if (fieldValue instanceof Map) {
      sb.append("{");
      for (Map.Entry<?, ?> entry : (((Map<?, ?>) fieldValue).entrySet())) {
        Object key = (Object) entry.getKey();
        Object val = (Object) entry.getValue();
        sb.append(newLine);
        sb.append(indentStr);
        sb.append(toString(key, indent + 2, prettyPrint));
        sb.append(space);
        sb.append(":");
        sb.append(space);
        sb.append(toString(val, indent + 2, prettyPrint));
      }
      sb.append(newLine + reduceIndent(indentStr) + "}");
    } else if (fieldValue instanceof java.util.Collection) {
      sb.append("[");
      for (Object o : (java.util.Collection<?>) fieldValue) {
        sb.append(newLine + indentStr + toString(o, indent + 2, prettyPrint));
      }
      sb.append(newLine + reduceIndent(indentStr) + "]");
    } else if (fieldValue instanceof TUnion<?>) {
      sb.append(((TUnion<?>) fieldValue).toString(indent, prettyPrint));
    } else if (fieldValue instanceof TBase) {
      toStringHelper(sb, (TBase) fieldValue, indent, prettyPrint);
    } else if (fieldValue instanceof String) {
      if (prettyPrint) {
        sb.append("\"" + fieldValue + "\"");
      } else {
        sb.append(fieldValue);
      }
    } else {
      sb.append(fieldValue.toString());
    }
  }

  /**
   * Set the value of a field based on its field id
   *
   * @param object The Thrift object to modified
   * @param fieldId The ID of the field to modify
   * @param value The value of the object to set
   */
  public static void setFieldValue(TBase object, short fieldId, Object value) {
    try {
      // For javadeprecated codegen, use the generated method
      Method setFieldValueMethod =
          object.getClass().getDeclaredMethod("setFieldValue", setFieldValueArgs);
      setFieldValueMethod.invoke(object, (int) fieldId, value);
    } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
      // android object are immutable
      throw new RuntimeException(
          String.format(
              "Failed to set the field value %s on %s (field id = %d): %s",
              value.toString(), object.getClass().getName(), fieldId, e.getMessage()));
    }
  }

  /**
   * Check is a field is set based on its field id
   *
   * @param object The Thrift object to modified
   * @param fieldId The ID of the field to modify
   */
  public static boolean isSet(TBase object, short fieldId) {
    try {
      TField tField = getTField(object, fieldId);
      String issetMethodName =
          "isSet" + tField.name.substring(0, 1).toUpperCase() + tField.name.substring(1);
      Method isset = object.getClass().getDeclaredMethod(issetMethodName);
      return (boolean) isset.invoke(object);
    } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
      return false;
    }
  }

  /**
   * Get the value of a field based on its field id
   *
   * @param object The Thrift object to modified
   * @param fieldId The ID of the field to modify
   */
  public static Object getFieldValue(TBase object, short fieldId) {
    try {
      try {
        // For javadeprecated codegen, use the generated method
        Method getFieldValueMethod =
            object.getClass().getDeclaredMethod("getFieldValue", int.class);
        return getFieldValueMethod.invoke(object, (int) fieldId);
      } catch (Exception e) {
        // android objects don't have an `getFieldValue` method
        TField tField = getTField(object, fieldId);
        String getterMethodName =
            "get" + tField.name.substring(0, 1).toUpperCase() + tField.name.substring(1);
        Method getter = object.getClass().getDeclaredMethod(getterMethodName);
        return getter.invoke(object);
      }
    } catch (Exception e) {
      throw new RuntimeException(
          String.format(
              "object %s doesn't seems to be a valid TBase Thrift object: %s",
              object.getClass().getName(), e.getMessage()));
    }
  }

  public static TField getTField(TBase object, short fieldId) {
    TField tField = null;
    try {
      for (Field field : object.getClass().getDeclaredFields()) {
        if (field.getType() == TField.class) {
          field.setAccessible(true);
          TField tf = (TField) field.get(object);
          if (tf.id == fieldId) {
            tField = tf;
            break;
          }
        }
      }
    } catch (IllegalAccessException e) {
      return null;
    }

    if (tField == null) {
      throw new IllegalArgumentException(
          String.format(
              "field #%d not found in Thrift object %s", fieldId, object.getClass().getName()));
    }
    return tField;
  }

  public static int getEnumValueOrZero(TEnum tEnum) {
    return getEnumValueOrDefault(tEnum, 0);
  }

  public static int getEnumValueOrDefault(TEnum tEnum, int defaultValue) {
    if (tEnum == null) {
      return defaultValue;
    }
    return tEnum.getValue();
  }
}
