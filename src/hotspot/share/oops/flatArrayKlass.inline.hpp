/*
 * Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */
#ifndef SHARE_VM_OOPS_FLATARRAYKLASS_INLINE_HPP
#define SHARE_VM_OOPS_FLATARRAYKLASS_INLINE_HPP

#include "oops/flatArrayKlass.hpp"

#include "memory/iterator.hpp"
#include "memory/memRegion.hpp"
#include "oops/arrayKlass.hpp"
#include "oops/flatArrayOop.hpp"
#include "oops/flatArrayOop.inline.hpp"
#include "oops/inlineKlass.hpp"
#include "oops/inlineKlass.inline.hpp"
#include "oops/klass.hpp"
#include "oops/oop.inline.hpp"
#include "utilities/devirtualizer.inline.hpp"
#include "utilities/macros.hpp"

/*
 * Warning incomplete: requires embedded oops, not yet enabled, so consider this a "sketch-up" of oop iterators
 */

template <typename T, class OopClosureType>
void FlatArrayKlass::oop_oop_iterate_elements_specialized(flatArrayOop a,
                                                          OopClosureType* closure) {
  oop_oop_iterate_elements_range_specialized<T>(a, closure, 0, a->length());
}

template <typename T, class OopClosureType>
void FlatArrayKlass::oop_oop_iterate_elements_range_specialized(flatArrayOop a,
                                                                OopClosureType* closure,
                                                                int start, int end) {
  precond(contains_oops());
  precond(start >= 0);
  assert(start <= end, "Invalid range [%d - %d)", start, end);
  assert(end <= a->length(), "Invalid range [%d - %d) for a.length: %d", start, end, a->length());

  const address base = (address)a->base();
  const int shift = Klass::layout_helper_log2_element_size(layout_helper());

  for (int index = start; index < end; index++) {
    element_klass()->oop_iterate_specialized<T>(base + (index << shift), closure);
  }
}

template <typename T, class OopClosureType>
void FlatArrayKlass::oop_oop_iterate_elements_specialized_bounded(flatArrayOop a,
                                                                  OopClosureType* closure,
                                                                  uintptr_t low, uintptr_t high) {
  assert(contains_oops(), "Nothing to iterate");

  const int shift = Klass::layout_helper_log2_element_size(layout_helper());
  const int size = 1 << shift;

  uintptr_t p = (uintptr_t)a->base();
  uintptr_t end = p + ((size_t)a->length() << shift);

  // The code aligns p and end to be at the boundaries of the first and last
  // element in the range. InlineKlass::oop_iterate_specialized_bounded will
  // perform the final filtering of the range.

  if (p < low) {
    uintptr_t diff = low - p;
    p += (diff >> shift) << shift;
  }

  if (end > high) {
    uintptr_t diff = end - high;
    end -= (diff >> shift) << shift;
  }

  for (; p < end; p += size) {
    element_klass()->oop_iterate_specialized_bounded<T>((address)p, closure, low, high);
  }
}

template <typename T, class OopClosureType>
void FlatArrayKlass::oop_oop_iterate_elements(flatArrayOop a, OopClosureType* closure) {
  if (contains_oops()) {
    oop_oop_iterate_elements_specialized<T>(a, closure);
  }
}

template <typename T, typename OopClosureType>
void FlatArrayKlass::oop_oop_iterate(oop obj, OopClosureType* closure) {
  assert(obj->is_flatArray(), "must be a flat array");
  flatArrayOop a = flatArrayOop(obj);

  if (Devirtualizer::do_metadata(closure)) {
    Devirtualizer::do_klass(closure, obj->klass());
  }

  oop_oop_iterate_elements<T>(a, closure);
}

template <typename T, typename OopClosureType>
void FlatArrayKlass::oop_oop_iterate_reverse(oop obj, OopClosureType* closure) {
  // TODO
  oop_oop_iterate<T>(obj, closure);
}

template <typename T, class OopClosureType>
void FlatArrayKlass::oop_oop_iterate_elements_bounded(flatArrayOop a, OopClosureType* closure, MemRegion mr) {
  if (contains_oops()) {
    oop_oop_iterate_elements_specialized_bounded<T>(a, closure, (uintptr_t)mr.start(), (uintptr_t)mr.end());
  }
}

template <typename T, typename OopClosureType>
void FlatArrayKlass::oop_oop_iterate_bounded(oop obj, OopClosureType* closure, MemRegion mr) {
  flatArrayOop a = flatArrayOop(obj);
  if (Devirtualizer::do_metadata(closure)) {
    Devirtualizer::do_klass(closure, a->klass());
  }
  oop_oop_iterate_elements_bounded<T>(a, closure, mr);
}

// Like oop_oop_iterate but only iterates over the specified range [start, end)
template <typename T, class OopClosureType>
void FlatArrayKlass::oop_oop_iterate_elements_range(flatArrayOop a, OopClosureType *closure, int start, int end) {
  if (contains_oops()) {
    oop_oop_iterate_elements_range_specialized<T>(a, closure, start, end);
  }
}

#endif // SHARE_VM_OOPS_FLATARRAYKLASS_INLINE_HPP
