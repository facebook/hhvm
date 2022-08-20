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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

/**
 * IntRangeSet is a specialized Set<Integer> implementation designed specifically to make the
 * generated validate() method calls faster. It groups the set values into ranges, and in the
 * contains() call, it does num ranges * 2 comparisons max. For the common case, which is a single,
 * contiguous range, this approach is about 60% faster than using a HashSet. If you had a very
 * ragged value set, like all the odd numbers, for instance, then you would end up with pretty poor
 * running time.
 */
public class IntRangeSet implements Set<Integer> {
  /**
   * This array keeps the bounds of each extent in alternating cells, always increasing. Example:
   * [0,5,10,15], which corresponds to 0-5, 10-15.
   */
  private int[] extents;

  /**
   * We'll keep a duplicate, real HashSet around internally to satisfy some of the other set
   * operations.
   */
  private Set<Integer> realSet = new HashSet<Integer>();

  public IntRangeSet(int... values) {
    Arrays.sort(values);

    List<Integer> extent_list = new ArrayList<Integer>();

    int ext_start = values[0];
    int ext_end_so_far = values[0];
    realSet.add(values[0]);
    for (int i = 1; i < values.length; i++) {
      realSet.add(values[i]);

      if (values[i] == ext_end_so_far + 1) {
        // advance the end so far
        ext_end_so_far = values[i];
      } else {
        // create an extent for everything we saw so far, move on to the next one
        extent_list.add(ext_start);
        extent_list.add(ext_end_so_far);
        ext_start = values[i];
        ext_end_so_far = values[i];
      }
    }
    extent_list.add(ext_start);
    extent_list.add(ext_end_so_far);

    extents = new int[extent_list.size()];
    for (int i = 0; i < extent_list.size(); i++) {
      extents[i] = extent_list.get(i);
    }
  }

  public boolean add(Integer i) {
    throw new UnsupportedOperationException();
  }

  public void clear() {
    throw new UnsupportedOperationException();
  }

  public boolean addAll(Collection<? extends Integer> arg0) {
    throw new UnsupportedOperationException();
  }

  /**
   * While this method is here for Set interface compatibility, you should avoid using it. It incurs
   * boxing overhead! Use the int method directly, instead.
   */
  public boolean contains(Object arg0) {
    return contains(((Integer) arg0).intValue());
  }

  /**
   * This is much faster, since it doesn't stop at Integer on the way through.
   *
   * @param val the value you want to check set membership for
   * @return true if val was found, false otherwise
   */
  public boolean contains(int val) {
    for (int i = 0; i < extents.length / 2; i++) {
      if (val < extents[i * 2]) {
        return false;
      } else if (val <= extents[i * 2 + 1]) {
        return true;
      }
    }

    return false;
  }

  public boolean containsAll(Collection<?> arg0) {
    for (Object o : arg0) {
      if (!contains(o)) {
        return false;
      }
    }
    return true;
  }

  public boolean isEmpty() {
    return realSet.isEmpty();
  }

  public Iterator<Integer> iterator() {
    return realSet.iterator();
  }

  public boolean remove(Object arg0) {
    throw new UnsupportedOperationException();
  }

  public boolean removeAll(Collection<?> arg0) {
    throw new UnsupportedOperationException();
  }

  public boolean retainAll(Collection<?> arg0) {
    throw new UnsupportedOperationException();
  }

  public int size() {
    return realSet.size();
  }

  public Object[] toArray() {
    return realSet.toArray();
  }

  public <T> T[] toArray(T[] arg0) {
    return realSet.toArray(arg0);
  }

  @Override
  public String toString() {
    String buf = "";
    for (int i = 0; i < extents.length / 2; i++) {
      if (i != 0) {
        buf += ", ";
      }
      buf += "[" + extents[i * 2] + "," + extents[i * 2 + 1] + "]";
    }
    return buf;
  }
}
