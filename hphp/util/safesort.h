/**
 * ===========================================================================
 * libc++ License
 * ===========================================================================
 *
 * The libc++ library is dual licensed under both the University of Illinois
 * "BSD-Like" license and the MIT license. As a user of this code you may
 * choose to use it under either license. As a contributor, you agree to allow
 * your code to be used under both.
 *
 * Full text of the relevant licenses is included below.
 *
 * ===========================================================================
 *
 * University of Illinois/NCSA
 * Open Source License
 *
 * Copyright (c) 2009-2012 by the contributors listed at
 * http://llvm.org/svn/llvm-project/libcxx/trunk/CREDITS.TXT
 *
 * All rights reserved.
 *
 * Developed by:
 *
 *     LLVM Team
 *
 *     University of Illinois at Urbana-Champaign
 *
 *     http://llvm.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimers.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimers in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the names of the LLVM Team, University of Illinois at
 *       Urbana-Champaign, nor the names of its contributors may be used to
 *       endorse or promote products derived from this Software without
 *       specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 *
 * ===========================================================================
 *
 * Copyright (c) 2009-2012 by the contributors listed at
 * http://llvm.org/svn/llvm-project/libcxx/trunk/CREDITS.TXT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * == Safesort ==
 *
 * The safesort algorithm below is based on LLVM's libc++ implementation
 * of std::sort.
 *
 * One key difference is that safesort is safe to use with a comparator
 * that does not impose a strict weak ordering on the elements (whereas
 * std::sort may crash or go into infinite loops for such comparators).
 * Safesoft is also "exception safe", leaving the array in a consistent
 * state in the event that the comparator throws. This is important for
 * HHVM for several reasons. Some of the builtin comparators in PHP do
 * not impose a strict weak ordereding (ex. SORT_REGULAR over strings).
 * Also, user code can supply comparators that behave inconsistently or
 * throw exceptions.
 *
 * In cases where the comparator does not impose a strict weak ordering
 * or the comparator throws, no guarantees are made about the order of
 * the elements produced the sort algorithm, though the algorithm still
 * upholds a weaker guarantee that the result will be some permutation
 * of the input.
 *
 * Another import difference is that safesort assumes the comparator
 * returns true if the left argument is GREATER than the right argument.
 * This is the opposite of what the STL's sort implementation does, and
 * we do it because it helps HHVM be more compatible with existing PHP
 * programs that (inadvertently) depend on unspecified behavior of the
 * PHP5 implementation.
 */

#pragma once

#include <algorithm>

namespace HPHP {
namespace Sort {

template <class GtCompT, class IterT>
void sort3(IterT x, IterT y, IterT z, GtCompT gt) {
  using std::swap;
  if (!gt(*x, *y)) {
    if (!gt(*y, *z))
      return;
    swap(*y, *z);
    if (gt(*x, *y)) {
      swap(*x, *y);
    }
    return;
  }
  if (gt(*y, *z)) {
    swap(*x, *z);
    return;
  }
  swap(*x, *y);
  if (gt(*y, *z)) {
    swap(*y, *z);
  }
}

template <class GtCompT, class IterT>
void sort4(IterT x1, IterT x2, IterT x3, IterT x4, GtCompT gt) {
  using std::swap;
  sort3<GtCompT>(x1, x2, x3, gt);
  if (gt(*x3, *x4)) {
    swap(*x3, *x4);
    if (gt(*x2, *x3)) {
      swap(*x2, *x3);
      if (gt(*x1, *x2)) {
        swap(*x1, *x2);
      }
    }
  }
}

template <class GtCompT, class IterT>
void sort5(IterT x1, IterT x2, IterT x3, IterT x4, IterT x5, GtCompT gt) {
  using std::swap;
  sort4<GtCompT>(x1, x2, x3, x4, gt);
  if (gt(*x4, *x5)) {
    swap(*x4, *x5);
    if (gt(*x3, *x4)) {
      swap(*x3, *x4);
      if (gt(*x2, *x3)) {
        swap(*x2, *x3);
        if (gt(*x1, *x2)) {
          swap(*x1, *x2);
        }
      }
    }
  }
}

template <class GtCompT, class IterT>
void insertion_sort(IterT first, IterT last, GtCompT gt) {
  typedef typename std::iterator_traits<IterT>::value_type value_type;
  typedef typename std::iterator_traits<IterT>::difference_type
    difference_type;
  difference_type len = last - first;
  if (len < 2) {
    // If there aren't at least 2 elements, we're done
    return;
  }
  // Loop over the first six elements
  IterT i = first;
  ++i;
  IterT l = (len < 6) ? last : first+6;
  for (; i != l; ++i) {
    IterT j = i;
    --j;
    // If this element is not less than the element
    // immediately before it, then we can leave this
    // element where it is for now
    if (!gt(*j, *i))
      continue;
    // Scan backward one element at a time looking
    // for the earliest element that *i is less than
    for (;;) {
      if (j == first) {
        break;
      }
      IterT k = j;
      --k;
      if (!gt(*k, *i)) {
        break;
      }
      j = k;
    }
    value_type t(*i);
    for (IterT k = i; k != j; --k) {
      *k = *(k-1);
    }
    *j = t;
  }
  // Loop over the remaining elements
  IterT second = first;
  ++second;
  for (; i != last; ++i) {
    IterT j = i;
    --j;
    // If this element is not less than the element
    // immediately before it, then we can leave this
    // element where it is for now
    if (!gt(*j, *i))
      continue;
    // Scan backward two elements at a time looking
    // for the earliest element that *i is less than
    for (;;) {
      // Invariant: j >= first && *i < *j
      if (j <= second) {
        // j points to first or second, so we have
        // reached the end of the loop
        if (j == second) {
          // If j points to second, we need to test
          // if *i is less than *first
          IterT m = j;
          --m;
          if (gt(*m, *i)) {
            j = m;
          }
        }
        break;
      }
      // Move backward by two
      IterT k = j-2;
      if (!gt(*k, *i)) {
        // If (*i < *k) is false, we know that *(k+1) or
        // *(k+2) is the element we are looking for.
        IterT m = k;
        ++m;
        if (gt(*m, *i)) {
          j = m;
        }
        break;
      }
      j = k;
    }
    // Move *i to temporary t, move the elements in the
    // range [j,i) over to the right one position, and
    // then move t to *j
    value_type t(*i);
    for (IterT m = i; m != j; --m) {
      *m = *(m-1);
    }
    *j = t;
  }
}

template <class GtCompT, class IterT>
void sort(IterT first, IterT last, GtCompT gt) {
  typedef typename std::iterator_traits<IterT>::difference_type
    difference_type;
  using std::swap;
  while (true) {
    difference_type len = last - first;
    // For small numbers of elements, use insertion sort
    if (len <= 16) {
      insertion_sort<GtCompT>(first, last, gt);
      return;
    }
    // Find a pivot
    IterT pivot;
    {
      IterT lm1 = last-1;
      difference_type delta = len/2;
      pivot = first + delta;
      if (len >= 1000) {
        // Compute the median of 5
        delta /= 2;
        sort5<GtCompT>(first, first + delta, pivot, pivot+delta, lm1, gt);
      } else {
        // Compute the median of 3
        sort3<GtCompT>(first, pivot, lm1, gt);
      }
      // Temporarily move the pivot to the second position
      swap(*(first+1), *pivot);
      pivot = first+1;
    }
    // Split the elements into two partitions (excluding the pivot);
    // we don't have to inspect the first element and last element
    // because they've already been put in the right place by the
    // call to sort3/sort5 above
    IterT i = first+2;
    IterT j = last-1;
    for (;;) {
      while (gt(*pivot, *i)) {
        ++i;
        if (UNLIKELY(i == j)) {
          goto done;
        }
      }
      --j;
      if (UNLIKELY(i == j)) {
        goto done;
      }
      while (gt(*j, *pivot)) {
        --j;
        if (UNLIKELY(i == j)) {
          goto done;
        }
      }
      swap(*i, *j);
      ++i;
      if (UNLIKELY(i == j)) {
        goto done;
      }
    }
    done:
    // Put the pivot in between the left partition and right partition
    swap(*pivot, *(i-1));
    // We now have the left partition in [first,i-1) and we have the
    // right parition in [i,last). Sort smaller partition with recursive
    // call and sort the larger partition with tail recursion elimination
    if ((i-1) - first < last - i) {
      sort<GtCompT>(first, i-1, gt);
      first = i;
    } else {
      sort<GtCompT>(i, last, gt);
      last = i-1;
    }
  }
}

}
}


