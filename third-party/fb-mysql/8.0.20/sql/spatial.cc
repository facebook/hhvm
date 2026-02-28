/*
   Copyright (c) 2002, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/spatial.h"

#include <algorithm>
#include <cmath>  // isfinite
#include <map>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "my_sys.h"
#include "myisampack.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/check_stack.h"  // check_stack_overrun
#include "sql/current_thd.h"
#include "sql/gis/srid.h"
#include "sql/gis_bg_traits.h"  // IWYU pragma: keep
#include "sql/gstream.h"        // Gis_read_stream
#include "sql/psi_memory_key.h"
#include "sql/sql_const.h"   // STACK_MIN_SIZE
#include "sql_string.h"      // String
#include "template_utils.h"  // pointer_cast
#include "unsafe_string_append.h"

void *gis_wkb_alloc(size_t sz) {
  sz += GEOM_HEADER_SIZE;
  char *p = static_cast<char *>(
      my_malloc(key_memory_Geometry_objects_data, sz, MYF(MY_FAE)));
  p += GEOM_HEADER_SIZE;
  return p;
}

void *gis_wkb_realloc(void *p, size_t sz) {
  char *cp = static_cast<char *>(p);
  if (cp) cp -= GEOM_HEADER_SIZE;
  sz += GEOM_HEADER_SIZE;

  p = my_realloc(key_memory_Geometry_objects_data, cp, sz, MYF(MY_FAE));
  cp = static_cast<char *>(p);
  return cp + GEOM_HEADER_SIZE;
}

/***************************** MBR *******************************/

/*
  Returns 0/1 if this MBR doesn't/does touch mbr. Returns -1 if the MBRs
  contain invalid data. This convention is true for all MBR relation test
  functions.
*/
int MBR::touches(const MBR *mbr) const {
  const MBR *mbr2 = mbr;
  const MBR *mbr1 = this;
  int ret = 0;
  int dim1 = dimension();
  int dim2 = mbr->dimension();

  DBUG_ASSERT(dim1 >= 0 && dim1 <= 2 && dim2 >= 0 && dim2 <= 2);
  if (dim1 == 0 && dim2 == 0) return 0;
  if (dim1 == 0 && dim2 == 1)
    return ((mbr1->xmin == mbr2->xmin && mbr1->ymin == mbr2->ymin) ||
            (mbr1->xmin == mbr2->xmax && mbr1->ymin == mbr2->ymax));
  if (dim1 == 1 && dim2 == 0) return mbr->touches(this);

  DBUG_ASSERT(dim1 + dim2 >= 2);
  ret = ((mbr2->xmin == mbr1->xmax || mbr2->xmax == mbr1->xmin) &&
         (mbr1->ymin <= mbr2->ymax && mbr1->ymax >= mbr2->ymin)) ||
        ((mbr2->ymin == mbr1->ymax || mbr2->ymax == mbr1->ymin) &&
         (mbr1->xmin <= mbr2->xmax && mbr1->xmax >= mbr2->xmin));

  if (ret && dim1 == 1 && dim2 == 1) {
    // The two line segments may overlap, rather than touch.
    int overlaps = ((mbr1->ymin == mbr1->ymax && mbr1->ymin == mbr2->ymax &&
                     mbr2->ymin == mbr2->ymax && mbr1->xmin < mbr2->xmax &&
                     mbr1->xmax > mbr2->xmin) ||
                    (mbr1->xmin == mbr1->xmax && mbr2->xmin == mbr2->xmax &&
                     mbr1->xmin == mbr2->xmin && mbr1->ymin < mbr2->ymax &&
                     mbr1->ymax > mbr2->ymin));
    if (overlaps) ret = 0;
  }

  return ret;
}

int MBR::within(const MBR *mbr) const {
  int dim1 = dimension();
  int dim2 = mbr->dimension();

  DBUG_ASSERT(dim1 >= 0 && dim1 <= 2 && dim2 >= 0 && dim2 <= 2);

  /*
    Either/both of the two operands can degrade to a point or a
    horizontal/vertical line segment, and we have to treat such cases
    separately.
   */
  switch (dim1) {
    case 0:
      DBUG_ASSERT(xmin == xmax && ymin == ymax);
      switch (dim2) {
        case 0:
          DBUG_ASSERT(mbr->xmin == mbr->xmax && mbr->ymin == mbr->ymax);
          return equals(mbr);
          break;
        case 1:
          DBUG_ASSERT((mbr->xmin == mbr->xmax && mbr->ymin != mbr->ymax) ||
                      (mbr->ymin == mbr->ymax && mbr->xmin != mbr->xmax));
          return ((xmin > mbr->xmin && xmin < mbr->xmax && ymin == mbr->ymin) ||
                  (ymin > mbr->ymin && ymin < mbr->ymax && xmin == mbr->xmin));
          break;
        case 2:
          DBUG_ASSERT(mbr->xmin != mbr->xmax && mbr->ymin != mbr->ymax);
          return (xmin > mbr->xmin && xmax < mbr->xmax && ymin > mbr->ymin &&
                  ymax < mbr->ymax);
          break;
      }
      break;
    case 1:
      DBUG_ASSERT((xmin == xmax && ymin != ymax) ||
                  (ymin == ymax && xmin != xmax));
      switch (dim2) {
        case 0:
          DBUG_ASSERT(mbr->xmin == mbr->xmax && mbr->ymin == mbr->ymax);
          return 0;
          break;
        case 1:
          DBUG_ASSERT((mbr->xmin == mbr->xmax && mbr->ymin != mbr->ymax) ||
                      (mbr->ymin == mbr->ymax && mbr->xmin != mbr->xmax));
          return (
              (xmin == xmax && mbr->xmin == mbr->xmax && mbr->xmin == xmin &&
               mbr->ymin <= ymin && mbr->ymax >= ymax) ||
              (ymin == ymax && mbr->ymin == mbr->ymax && mbr->ymin == ymin &&
               mbr->xmin <= xmin && mbr->xmax >= xmax));
          break;
        case 2:
          DBUG_ASSERT(mbr->xmin != mbr->xmax && mbr->ymin != mbr->ymax);
          return ((xmin == xmax && xmin > mbr->xmin && xmax < mbr->xmax &&
                   ymin >= mbr->ymin && ymax <= mbr->ymax) ||
                  (ymin == ymax && ymin > mbr->ymin && ymax < mbr->ymax &&
                   xmin >= mbr->xmin && xmax <= mbr->xmax));
          break;
      }
      break;
    case 2:
      DBUG_ASSERT(xmin != xmax && ymin != ymax);
      switch (dim2) {
        case 0:
        case 1:
          return 0;
          break;
        case 2:
          DBUG_ASSERT(mbr->xmin != mbr->xmax && mbr->ymin != mbr->ymax);
          return ((mbr->xmin <= xmin) && (mbr->ymin <= ymin) &&
                  (mbr->xmax >= xmax) && (mbr->ymax >= ymax));
          break;
      }
      break;
  }

  // Never reached.
  DBUG_ASSERT(false);
  return 0;
}

/*
  exponential notation :
  1   sign
  1   number before the decimal point
  1   decimal point
  17  number of significant digits (see String::qs_append(double))
  1   'e' sign
  1   exponent sign
  3   exponent digits
  ==
  25

  "f" notation :
  1   optional 0
  1   sign
  17  number significant digits (see String::qs_append(double) )
  1   decimal point
  ==
  20
*/

#define MAX_DIGITS_IN_DOUBLE 25

/**
  Distance to another point.
*/
double point_xy::distance(const point_xy &p) const {
  /* On 32bit platforms, sqrt(inf) may produce a wrong number that isn't inf. */
  const double a = pow(x - p.x, 2.0);
  if (!std::isfinite(a)) return a;
  const double b = pow(y - p.y, 2.0);
  if (!std::isfinite(a + b)) return a + b;
  return sqrt(a + b);
}

/***************************** Gis_class_info *******************************/

String Geometry::bad_geometry_data("Bad object", &my_charset_bin);

Geometry::Class_info *Geometry::ci_collection[Geometry::wkb_last + 1] = {
    nullptr};

static Geometry::Class_info **ci_collection_end =
    Geometry::ci_collection + Geometry::wkb_last + 1;

Geometry::Class_info::Class_info(const char *name, int type_id,
                                 create_geom_t create_func)
    : m_name{name, strlen(name)},
      m_type_id(type_id),
      m_create_func(create_func) {
  ci_collection[type_id] = this;
}

inline static Geometry *create_point(char *buffer) {
  return ::new (buffer) Gis_point(false);
}

inline static Geometry *create_linestring(char *buffer) {
  return ::new (buffer) Gis_line_string(false);
}

inline static Geometry *create_polygon(char *buffer) {
  return ::new (buffer) Gis_polygon(false);
}

inline static Geometry *create_multipoint(char *buffer) {
  return ::new (buffer) Gis_multi_point(false);
}

inline static Geometry *create_multipolygon(char *buffer) {
  return ::new (buffer) Gis_multi_polygon(false);
}

inline static Geometry *create_multilinestring(char *buffer) {
  return ::new (buffer) Gis_multi_line_string(false);
}

inline static Geometry *create_geometrycollection(char *buffer) {
  return ::new (buffer) Gis_geometry_collection();
}

/**
  Check if geometry type sub is a subtype of super.

  Since Geometry::wkbType can't represent the geometry type, the
  superclass of all geometry types, this function can't check
  that. The supertype has to be a subtype of geometry.

  @param sub The type to check
  @param super The supertype

  @return True if t1 is a subtype of t2
 */
inline static bool is_subtype_of(Geometry::wkbType sub,
                                 Geometry::wkbType super) {
  return (super == Geometry::wkb_geometrycollection &&
          (sub == Geometry::wkb_multipoint ||
           sub == Geometry::wkb_multilinestring ||
           sub == Geometry::wkb_multipolygon));
}

static Geometry::Class_info point_class("POINT", Geometry::wkb_point,
                                        create_point);

static Geometry::Class_info linestring_class("LINESTRING",
                                             Geometry::wkb_linestring,
                                             create_linestring);
static Geometry::Class_info polygon_class("POLYGON", Geometry::wkb_polygon,
                                          create_polygon);
static Geometry::Class_info multipoint_class("MULTIPOINT",
                                             Geometry::wkb_multipoint,
                                             create_multipoint);
static Geometry::Class_info multilinestring_class("MULTILINESTRING",
                                                  Geometry::wkb_multilinestring,
                                                  create_multilinestring);
static Geometry::Class_info multipolygon_class("MULTIPOLYGON",
                                               Geometry::wkb_multipolygon,
                                               create_multipolygon);
static Geometry::Class_info geometrycollection_class(
    "GEOMCOLLECTION", Geometry::wkb_geometrycollection,
    create_geometrycollection);

/***************************** Geometry *******************************/

Geometry::Class_info *Geometry::find_class(const char *name, size_t len) {
  for (Class_info **cur_rt = ci_collection; cur_rt < ci_collection_end;
       cur_rt++) {
    if (*cur_rt && (*cur_rt)->m_type_id == Geometry::wkb_geometrycollection) {
      if (len == 18 &&
          my_strnncoll(&my_charset_latin1,
                       pointer_cast<const uchar *>("GEOMETRYCOLLECTION"), len,
                       pointer_cast<const uchar *>(name), len) == 0)
        return *cur_rt;
    } else if (*cur_rt && ((*cur_rt)->m_name.length == len) &&
               (my_strnncoll(&my_charset_latin1,
                             pointer_cast<const uchar *>((*cur_rt)->m_name.str),
                             len, pointer_cast<const uchar *>(name), len) == 0))
      return *cur_rt;
  }
  return nullptr;
}

Geometry *Geometry::create_by_typeid(Geometry_buffer *buffer, int type_id) {
  Class_info *ci;
  if (!(ci = find_class(type_id))) return nullptr;
  return (*ci->m_create_func)(buffer->data);
}

/**
  Construct a Geometry object using GEOMETRY byte string. This function is
  called by all GIS functions to make a Geometry object from a GEOMETRY byte
  string, which can come from table storage, or returned from other GIS
  function, or directly provided by user via client.

  The WKB can be of either endianess --- when user directly pass WKB
  byte string to us, he/she can pass big endian WKB, otherwise the WKB is
  always little endian. And we should reject big endian WKB because all the
  rest of the GIS code assumes the internal WKB data being always little endian.

  @param buffer The place where the Geometry object is constructed on.
  @param data is a byte string with an optional srid prepending a WKB format
  byte string, which is called a GEOMETRY byte string and which is the inner
  storage format of all geometries in MySQL.
  @param data_len number of bytes of the byte string refered by data.
  @param has_srid whether data argument starts with an srid or not.
  By default it's true, if false, data starts with WKB header, and caller
  is responsible to specify an srid to this object later.
  @return Constructed geometry object.
 */
Geometry *Geometry::construct(Geometry_buffer *buffer, const char *data,
                              uint32 data_len, bool has_srid) {
  uint32 geom_type;
  Geometry *result;
  wkbByteOrder bo;
  String wkb_le;
  uint32 srid_sz = has_srid ? SRID_SIZE : 0;
  // Check the GEOMETRY byte string is valid, which would at least have an
  // SRID, a WKB header, and 4 more bytes for object count or Point
  // coordinate.
  if (data_len < srid_sz + WKB_HEADER_SIZE + 4) return nullptr;

  bo = ::get_byte_order(data + srid_sz);

  if (bo != Geometry::wkb_ndr) {
    my_error(ER_GIS_DATA_WRONG_ENDIANESS, MYF(0));
    return nullptr;
    /*
      Don't try to convert endianess but error out because we can't
      replace the bytes refered by data, it can be from any source.
      Users can call GeometryFromWKB to use WKB of either endianess
      if they have to pass WKB/Geometry byte string from client to us.
     */
  }

  /* + 1 to skip the byte order (stored in position SRID_SIZE). */
  geom_type = uint4korr(data + srid_sz + 1);
  if (geom_type < wkb_first || geom_type > wkb_last ||
      !(result = create_by_typeid(buffer, (int)geom_type)))
    return nullptr;

  gis::srid_t srid = 0;
  if (has_srid) {
    srid = uint4korr(data);
    result->set_srid(srid);
  }

  if (geom_type == wkb_point) {
    if (data_len - srid_sz - WKB_HEADER_SIZE < POINT_DATA_SIZE) return nullptr;
    result->set_data_ptr(data + srid_sz + WKB_HEADER_SIZE, POINT_DATA_SIZE);
  } else
    result->set_data_ptr(data + srid_sz + WKB_HEADER_SIZE,
                         data_len - srid_sz - WKB_HEADER_SIZE);
  result->has_geom_header_space(has_srid);
  if (result->get_geotype() == wkb_polygon) result->polygon_is_wkb_form(true);

  /*
    Check whether the GEOMETRY byte string is a valid and complete one.
    Do not allow extra trailing bytes if this is a GEOMETRY byte string,
    otherwise we are creating a geometry object using part of the byte string
    which is longer than we need here but are not trash bytes.
  */
  const uint32 result_len = result->get_data_size();
  const uint32 header_size = (has_srid ? GEOM_HEADER_SIZE : WKB_HEADER_SIZE);
  if (result_len == GET_SIZE_ERROR ||
      (has_srid && (result_len + header_size) != data_len))
    return nullptr;

  return result;
}

/**
  Read wkt text from trs, and write little endian wkb encoding into 'wkt',
  and create a Geometry instance in 'buffer'. If 'init_stream' is true,
  shallow assign data in 'wkt' to the Geometry object to be returned.
  @param buffer Place to create the returned Geometry object at.
  @param trs WKT read stream.
  @param wkb Little endian WKB buffer for WKB data of the returned Geometry
  object.
  @param init_stream Whether set WKB buffer pointer to returned Geometry
  object.
  @param check_trailing
  @return A Geometry object with data specified by the WKT.
 */
Geometry *Geometry::create_from_wkt(Geometry_buffer *buffer,
                                    Gis_read_stream *trs, String *wkb,
                                    bool init_stream, bool check_trailing) {
  LEX_CSTRING name;
  Class_info *ci;

  if (trs->get_next_word(&name)) {
    trs->set_error_msg("Geometry name expected");
    return nullptr;
  }
  if (!(ci = find_class(name.str, name.length)) ||
      wkb->reserve(WKB_HEADER_SIZE, 512))
    return nullptr;
  Geometry *result = (*ci->m_create_func)(buffer->data);
  q_append((char)wkb_ndr, wkb);
  q_append((uint32)result->get_class_info()->m_type_id, wkb);

  if (result->init_from_wkt(trs, wkb) ||
      (check_trailing && !trs->is_end_of_stream()))
    return nullptr;

  if (init_stream)
    result->set_data_ptr(wkb->ptr() + WKB_HEADER_SIZE,
                         wkb->length() - WKB_HEADER_SIZE);
  result->has_geom_header_space(true);
  if (result->get_geotype() == wkb_polygon) result->polygon_is_wkb_form(true);

  return result;
}

/**
  Write this geometry's WKB byte string into specified buffer, the SRID is
  not written into the buffer.

  @param wkb The buffer to write WKB byte string into.
  @param shallow_copy Whether do shallow copy by using this object's memory
         without owning it or duplicating the byte string.
  @return true if got error, false if successful.
 */
bool Geometry::as_wkb(String *wkb, bool shallow_copy) const {
  DBUG_ASSERT(wkb->ptr() < get_cptr() - GEOM_HEADER_SIZE ||
              wkb->ptr() > get_cptr() + get_nbytes());

  if (shallow_copy) {
    /*
      This object must have GEOMETRY header space, and we simply assign to
      wkb, the memory is still owned by the String object of this Geometry
      object, i.e. the String object holding WKB data for this object.

      Don't write to this object's own String buffer.
     */
    DBUG_ASSERT(wkb->ptr() != get_cptr() - GEOM_HEADER_SIZE);

    DBUG_ASSERT(!(get_geotype() == wkb_polygon &&
                  (!polygon_is_wkb_form() || is_bg_adapter())));
    wkb->set(get_cptr() - WKB_HEADER_SIZE, get_nbytes() + WKB_HEADER_SIZE,
             &my_charset_bin);
    return false;
  }

  if (wkb->reserve(WKB_HEADER_SIZE + this->get_nbytes(), 512) ||
      get_data_ptr() == nullptr)
    return true;

  write_wkb_header(wkb, get_geotype());
  if (get_geotype() != wkb_polygon)
    q_append(static_cast<const char *>(this->get_data_ptr()),
             this->get_nbytes(), wkb);
  else {
    size_t len = 0;
    void *ptr = get_packed_ptr(this, &len);
    wkb->append(static_cast<char *>(ptr), len);
    gis_wkb_free(ptr);
  }

  return false;
}

/**
  Write this geometry's GEOMETRY byte string into specified buffer, the SRID
  will be written before the WKB string to form a GEOMETRY byte string.

  @param buf The buffer to write GEOMETRY byte string into.
  @param shallow_copy Whether do shallow copy by using this object's memory
         without owning it or duplicating the byte string.
  @return true if got error, false if successful.
*/
bool Geometry::as_geometry(String *buf, bool shallow_copy) const {
  if (shallow_copy) {
    /*
      This object must have GEOMETRY header space, and we simply assign to
      buf, the memory is still owned by the String object of this Geometry
      object, i.e. the String object holding WKB data for this object.

      Don't write to this object's own String buffer.
     */
    DBUG_ASSERT(has_geom_header_space());

    DBUG_ASSERT(!(get_geotype() == wkb_polygon &&
                  (!polygon_is_wkb_form() || is_bg_adapter())));

    if (buf->ptr() != get_cptr() - GEOM_HEADER_SIZE) {
      DBUG_ASSERT(buf->ptr() < get_cptr() - GEOM_HEADER_SIZE ||
                  buf->ptr() > get_cptr() + get_nbytes());
      buf->set(get_cptr() - GEOM_HEADER_SIZE, get_nbytes() + GEOM_HEADER_SIZE,
               &my_charset_bin);
    }
    return false;
  }

  if ((buf->ptr() == get_cptr() - GEOM_HEADER_SIZE)) {
    if (buf->is_alloced()) return false;
  } else
    DBUG_ASSERT(buf->ptr() < get_cptr() - GEOM_HEADER_SIZE ||
                buf->ptr() > get_cptr() + get_nbytes());

  if (buf->reserve(SRID_SIZE + WKB_HEADER_SIZE + this->get_nbytes(), 512) ||
      get_data_ptr() == nullptr)
    return true;

  write_geometry_header(buf, get_srid(), get_geotype());
  if (get_geotype() != wkb_polygon)
    q_append(static_cast<const char *>(this->get_data_ptr()),
             this->get_nbytes(), buf);
  else {
    size_t len = 0;
    void *ptr = get_packed_ptr(this, &len);
    buf->append(static_cast<char *>(ptr), len);
    gis_wkb_free(ptr);
  }
  return false;
}

/**
  WKB scanner event handler that checks if the WKB string is well formed.

  This doesn't check if the geometry is valid (e.g., it's not checking
  if a polygon is self-intersecting), it only checks some simple rules
  for WKB well-formedness:

  R1. The byte order is as specified (constructor parameter)
  R2. The wkbType is within the supported range
  R3. The geometry is of the specified type (constructor parameter),
      or a subtype
  R4. Nested geometries contain only geometries that can be contained
      by that type
  R5. Linestrings have at least two points
  R6. Polygon rings have at least four points
  R7. Polygons have at least one ring
  R8. Collections, except geometrycollection, contain at least one
      element.

  An additional requirement, that the WKB ends exactly at the end of
  the string, is checked by Geometry::is_well_formed(). The last parse
  position is maintained as last_position here to make that test
  possible.
 */
class Geometry_well_formed_checker : public WKB_scanner_event_handler {
 private:
  Prealloced_array<Geometry::wkbType, 8> type;  /// Current stack of types
  Geometry::wkbType previous_type;              /// Type of previous start/end
  int points_in_linestring;                     /// Points in current ls
  const void *last_position;                    /// Last wkb pointer seen
  bool is_ok;                                   /// Whether the WKB is OK so far
  Geometry::wkbByteOrder required_byte_order;

 public:
  /**
    Create a new even handler.

    @param type Expected geometry type. If set to
                Geometry::wkb_invalid_type, any geometry is allowed.
    @param required_byte_order
   */
  Geometry_well_formed_checker(Geometry::wkbType type,
                               Geometry::wkbByteOrder required_byte_order)
      : type(PSI_NOT_INSTRUMENTED),
        previous_type(Geometry::wkb_invalid_type),
        points_in_linestring(0),
        last_position(nullptr),
        is_ok(true),
        required_byte_order(required_byte_order) {
    this->type.push_back(type);
  }

  virtual void on_wkb_start(Geometry::wkbByteOrder bo,
                            Geometry::wkbType geotype, const void *, uint32,
                            bool has_hdr) {
    if (!is_ok) return;

    // The byte order must be the specified one (R1).
    if (required_byte_order != Geometry::wkb_invalid &&
        bo != required_byte_order) {
      is_ok = false;
      return;
    }

    Geometry::wkbType outer_type = type[type.size() - 1];

    type.push_back(geotype);
    previous_type = geotype;

    // The geometry type must be in the valid range (R2).
    if (geotype < Geometry::wkb_first ||
        geotype > Geometry::wkb_geometrycollection) {
      is_ok = false;
      return;
    }

    // First geometry must be of the specified type (if specified), or
    // a subtype (R3).
    if (type.size() == 2) {
      if (geotype != outer_type && outer_type != Geometry::wkb_invalid_type &&
          !is_subtype_of(geotype, outer_type))
        is_ok = false;
      return;
    }

    // Any type is allowed in geometry collections (R4).
    if (outer_type == Geometry::wkb_geometrycollection) return;

    switch (geotype) {
      case Geometry::wkb_point:
        // Points can only appear in multipoints and in linestrings (R4).
        if (!(outer_type == Geometry::wkb_multipoint ||
              (!has_hdr && outer_type == Geometry::wkb_linestring)))
          is_ok = false;
        if (outer_type == Geometry::wkb_linestring) ++points_in_linestring;
        break;
      case Geometry::wkb_linestring:
        // Linestrings can only appear in multilinestrings and as rings
        // in polygons (R4).
        if (!(outer_type == Geometry::wkb_multilinestring ||
              (!has_hdr && outer_type == Geometry::wkb_polygon)))
          is_ok = false;
        break;
      case Geometry::wkb_polygon:
        // Polygons can only appear in multipolygons (R4).
        if (outer_type != Geometry::wkb_multipolygon) is_ok = false;
        break;
      case Geometry::wkb_multipoint:
      case Geometry::wkb_multilinestring:
      case Geometry::wkb_multipolygon:
      case Geometry::wkb_geometrycollection:
        // These are only allowed if outer_type is geometry collection,
        // in which case they're handled before entering the switch (R4).
        is_ok = false;
        break;
      default:
        // The list of cases above should be complete (R2).
        DBUG_ASSERT(0);
        break;
    }
  }

  virtual void on_wkb_end(const void *wkb) {
    if (!is_ok) return;

    Geometry::wkbType current_type = type[type.size() - 1];
    type.pop_back();
    last_position = wkb;

    switch (current_type) {
      case Geometry::wkb_linestring:
        // Linestrings must have at least two points. Polygon rings must
        // have at least four points (R5, R6).
        if (points_in_linestring < 2 ||
            (type[type.size() - 1] == Geometry::wkb_polygon &&
             points_in_linestring < 4))
          is_ok = false;
        points_in_linestring = 0;
        break;
      case Geometry::wkb_polygon:
        // Polygons must have at least one ring (R7).
        if (previous_type != Geometry::wkb_linestring) is_ok = false;
        break;
      case Geometry::wkb_multipoint:
        // Multipoints must contain at least one point (R8).
        if (previous_type != Geometry::wkb_point) is_ok = false;
        break;
      case Geometry::wkb_multilinestring:
        // Multilinestrings must contain at least one linestring (R8).
        if (previous_type != Geometry::wkb_linestring) is_ok = false;
        break;
      case Geometry::wkb_multipolygon:
        // Multipolygons must contain at least one polygon (R8).
        if (previous_type != Geometry::wkb_polygon) is_ok = false;
        break;
      default:
        break;
    }

    previous_type = current_type;
  }

  virtual bool continue_scan() const { return is_ok; }

  /**
    Check if the parsed WKB was well-formed, as far as this handler
    knows.

    There may be other conditions that cause the object to not be
    well-formed.

    @see Geometry::is_well_formed

    @return True if the WKB was well-formed, false otherwise.
   */
  bool is_well_formed() { return is_ok; }

  /**
    Get the position after the last parsed byte.

    @return Pointer pointing to after the last parsed byte.
  */
  const char *get_last_position() {
    return static_cast<const char *>(last_position);
  }
};

bool Geometry::is_well_formed(const char *from, size_t length,
                              Geometry::wkbType type,
                              Geometry::wkbByteOrder bo) {
  bool is_well_formed = true;
  Geometry_well_formed_checker checker(type, bo);
  uint32 len = length - SRID_SIZE;

  if (length < GEOM_HEADER_SIZE) return false;

  is_well_formed = (wkb_scanner(current_thd, from + SRID_SIZE, &len, 0, true,
                                &checker) != nullptr);

  return (is_well_formed && checker.is_well_formed() &&
          checker.get_last_position() == from + length);
}

static double wkb_get_double(const char *ptr, Geometry::wkbByteOrder bo) {
  if (bo == Geometry::wkb_ndr)
    return float8get(ptr);
  else
    return mi_float8get(pointer_cast<const uchar *>(ptr));
}

/**
   Check that a pair of geographic coordinates are within the valid range.

   Checks if the coordinates are within the allowed range for geographic
   coordinates. Valid range for longitude and latitude coordinates in geographic
   spatial reference systems are (-180, 180) and [-90, 90] degrees,
   respectively.

   @param[in] x Longitude coordinate.
   @param[in] y Latitude coordinate.
   @param[in] srs_angular_unit Unit to radians conversion factor.
   @param[out] long_out_of_range Longitude is out of range.
   @param[out] lat_out_of_range Latitude is out of range.
   @param[out] out_of_range_value The value that is out of range.

   @retval false Coordinates are within allowed range.
   @retval true Coordinates are not within allowed range.
*/

static bool check_coordinate_range(double x, double y, double srs_angular_unit,
                                   bool *long_out_of_range,
                                   bool *lat_out_of_range,
                                   double *out_of_range_value) {
  // Check if longitude coordinates are within allowed range.
  if (x * srs_angular_unit <= -M_PI || x * srs_angular_unit > M_PI) {
    *long_out_of_range = true;
    *out_of_range_value = x;
    return true;
  }

  // Check if latitude coordinates are within allowed range.
  if (y * srs_angular_unit < -M_PI_2 || y * srs_angular_unit > M_PI_2) {
    *lat_out_of_range = true;
    *out_of_range_value = y;
    return true;
  }

  return false;
}

static uint32 wkb_get_uint(const char *ptr, Geometry::wkbByteOrder bo) {
  if (bo == Geometry::wkb_ndr)
    return uint4korr(ptr);
  else
    return mi_uint4korr(pointer_cast<const uchar *>(ptr));
}

/**
  Scan WKB byte string and notify WKB events by calling registered callbacks.
  @param wkb a little endian WKB byte string of 'len' bytes, with or
             without WKB header.
  @param[in] thd Thread context.
  @param [in,out] len remaining number of bytes of the wkb string.
  @param geotype the type of the geometry to be scanned.
  @param has_hdr whether the 'wkb' point to a WKB header or right after
                the header. If it is true, the
                'geotype' should be the same as the type in the header;
                otherwise, and we will use the type specified in WKB header.
  @param handler the registered WKB_scanner_event_handler object to be notified.
  @return the next byte after last valid geometry just scanned, or NULL on error
 */
const char *wkb_scanner(THD *thd, const char *wkb, uint32 *len, uint32 geotype,
                        bool has_hdr, WKB_scanner_event_handler *handler) {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr)) return nullptr;

  Geometry::wkbType gt;
  const char *q = nullptr;
  uint32 ngeos = 0, comp_type = 0, gtype = 0;
  bool comp_hashdr = false, done = false;

  if (has_hdr) {
    if (*len < WKB_HEADER_SIZE) return nullptr;  // Invalid WKB data.

    gtype = uint4korr(wkb + 1);
    // The geotype isn't used in this case.
    if (geotype != gtype && geotype != 0 /* unknown */) return nullptr;

    if ((*wkb != Geometry::wkb_ndr && *wkb != Geometry::wkb_xdr) ||
        gtype < Geometry::wkb_first || gtype > Geometry::wkb_last)
      return nullptr;

    gt = static_cast<Geometry::wkbType>(gtype);

    q = wkb + WKB_HEADER_SIZE;
    *len -= WKB_HEADER_SIZE;
    if (*len <= 0) return nullptr;
    handler->on_wkb_start(get_byte_order(wkb), gt, q, *len, true);
    if (!handler->continue_scan()) return nullptr;
  } else {
    DBUG_ASSERT(geotype >= Geometry::wkb_first &&
                geotype <= Geometry::wkb_last);
    q = wkb;
    gt = static_cast<Geometry::wkbType>(geotype);
    handler->on_wkb_start(Geometry::wkb_ndr, gt, q, *len, false);
    if (!handler->continue_scan()) return nullptr;
  }

  if (gt != Geometry::wkb_point) {
    if (*len < 4) return nullptr;
    ngeos = uint4korr(q);
    q += sizeof(uint32);
    *len -= 4;
  }

  switch (gt) {
    case Geometry::wkb_point:
      if (*len < POINT_DATA_SIZE) return nullptr;
      q += POINT_DATA_SIZE;
      *len -= POINT_DATA_SIZE;
      done = true;
      handler->on_wkb_end(q);
      if (!handler->continue_scan()) return nullptr;
      break;
    case Geometry::wkb_linestring:
      comp_type = Geometry::wkb_point;
      break;
    case Geometry::wkb_polygon:
      comp_type = Geometry::wkb_linestring;
      break;
    case Geometry::wkb_multipoint:
      comp_type = Geometry::wkb_point;
      comp_hashdr = true;
      break;
    case Geometry::wkb_multilinestring:
      comp_type = Geometry::wkb_linestring;
      comp_hashdr = true;
      break;
    case Geometry::wkb_multipolygon:
      comp_type = Geometry::wkb_polygon;
      comp_hashdr = true;
      break;
    case Geometry::wkb_geometrycollection:
      comp_hashdr = true;
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  if (!done && q != nullptr) {
    for (uint32 i = 0; i < ngeos; i++) {
      q = wkb_scanner(thd, q, len, comp_type, comp_hashdr, handler);
      if (q == nullptr) return nullptr;
    }
    handler->on_wkb_end(q);
    if (!handler->continue_scan()) return nullptr;
  }

  return q;
}

/**
  Read from 'wkb' (which contains WKB encoded in either endianess) the
  geometry data, and write WKB of returned Geometry object in little endianess
  into 'res', and also create geometry object on 'buffer' and return it.
  The returned Geometry object points to bytes (without WKB HEADER) in 'res'.

  @param thd Thread context.
  @param buffer the place to create the returned Geometry object at.
  @param wkb the input WKB buffer which contains WKB of either endianess.
  @param len the number of bytes of WKB in 'wkb'.
  @param res the buffer to write little endian WKB into.
  @param init_stream Whether set WKB buffer pointer to returned Geometry
  object.
  @return the created Geometry object.
 */
Geometry *Geometry::create_from_wkb(THD *thd, Geometry_buffer *buffer,
                                    const char *wkb, uint32 len, String *res,
                                    bool init_stream) {
  uint32 geom_type;
  Geometry *geom;

  if (len < WKB_HEADER_SIZE) return nullptr;

  geom_type = wkb_get_uint(wkb + 1, ::get_byte_order(wkb));
  if ((*wkb != wkb_xdr && *wkb != wkb_ndr) || geom_type < wkb_first ||
      geom_type > wkb_last ||
      !(geom = create_by_typeid(buffer, (int)geom_type)) ||
      res->reserve(WKB_HEADER_SIZE, 512))
    return nullptr;

  q_append((char)wkb_ndr, res);
  q_append(geom_type, res);

  uint tret =
      geom->init_from_wkb(thd, wkb + WKB_HEADER_SIZE, len - WKB_HEADER_SIZE,
                          ::get_byte_order(wkb), res);

  // The WKB string is invalid if it has trailing trash bytes.
  if (tret != len - WKB_HEADER_SIZE) return nullptr;

  if (init_stream)
    geom->set_data_ptr(res->ptr() + WKB_HEADER_SIZE,
                       res->length() - WKB_HEADER_SIZE);
  geom->has_geom_header_space(true);
  if (geom->get_geotype() == wkb_polygon) geom->polygon_is_wkb_form(true);
  return tret ? geom : nullptr;
}

bool Geometry::envelope(MBR *mbr) const {
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (get_mbr(mbr, &wkb)) return true;

  return false;
}

class GeomColl_component_counter : public WKB_scanner_event_handler {
 public:
  size_t num;

  GeomColl_component_counter() : num(0) {}

  virtual void on_wkb_start(Geometry::wkbByteOrder, Geometry::wkbType geotype,
                            const void *, uint32, bool) {
    if (geotype != Geometry::wkb_geometrycollection) num++;
  }

  virtual void on_wkb_end(const void *) {}
};

bool Geometry::envelope(String *result) const {
  MBR mbr;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (result->reserve(1 + 4 * 3 + SIZEOF_STORED_DOUBLE * 10)) return true;

  if (get_mbr(&mbr, &wkb)) {
    /*
      The geometry has no effective components in this branch, which is
      impossible for geometries other than geometry collections(GC).
      A GC may have empty nested GCs.
    */
    if (get_type() != wkb_geometrycollection) return true;

    uint32 num = uint4korr(get_cptr());
    if (num != 0) {
      GeomColl_component_counter counter;
      uint32 wkb_len = get_data_size();

      wkb_scanner(current_thd, get_cptr(), &wkb_len,
                  Geometry::wkb_geometrycollection, false, &counter);
      // Non-empty nested geometry collections.
      if (counter.num > 0) return true;
    }

    // An empty geometry collection's envelope is an empty geometry.
    write_wkb_header(result, wkb_geometrycollection, 0);
    return false;
  }

  q_append(static_cast<char>(wkb_ndr), result);

  int dim = mbr.dimension();
  if (dim < 0) return true;

  uint32 num_elems, num_elems2;

  if (dim == 0) {
    q_append(static_cast<uint32>(wkb_point), result);
    q_append(mbr.xmin, result);
    q_append(mbr.ymin, result);
  } else if (dim == 1) {
    q_append(static_cast<uint32>(wkb_linestring), result);
    num_elems = 2;
    q_append(num_elems, result);
    q_append(mbr.xmin, result);
    q_append(mbr.ymin, result);
    q_append(mbr.xmax, result);
    q_append(mbr.ymax, result);
  } else {
    q_append(static_cast<uint32>(wkb_polygon), result);
    num_elems = 1;
    q_append(num_elems, result);
    num_elems2 = 5;
    q_append(num_elems2, result);
    q_append(mbr.xmin, result);
    q_append(mbr.ymin, result);
    q_append(mbr.xmax, result);
    q_append(mbr.ymin, result);
    q_append(mbr.xmax, result);
    q_append(mbr.ymax, result);
    q_append(mbr.xmin, result);
    q_append(mbr.ymax, result);
    q_append(mbr.xmin, result);
    q_append(mbr.ymin, result);
  }
  return false;
}

/**
  Create a point from data.

  @param [out] result   Put result here
  @param wkb          Data for point is here.

  @return             false on success, true on error
*/

bool Geometry::create_point(String *result, wkb_parser *wkb) const {
  if (wkb->no_data(POINT_DATA_SIZE) ||
      result->reserve(WKB_HEADER_SIZE + POINT_DATA_SIZE, 32))
    return true;
  q_append((char)wkb_ndr, result);
  q_append((uint32)wkb_point, result);
  /* Copy two double in same format */
  q_append(wkb->data(), POINT_DATA_SIZE, result);
  return false;
}

/**
  Create a point from coordinates.

  @param [out] result
  @param p  coordinates for point

  @return  false on success, true on error
*/

bool Geometry::create_point(String *result, point_xy p) const {
  if (result->reserve(1 + 4 + POINT_DATA_SIZE, 32)) return true;

  q_append((char)wkb_ndr, result);
  q_append((uint32)wkb_point, result);
  q_append(p.x, result);
  q_append(p.y, result);
  return false;
}

/**
  Append N points from packed format to text
  Before calling this function, caller must have already checked that wkb's
  buffer is complete and not truncated.

  @param [out] txt        Append points here
  @param     n_points   Number of points
  @param     wkb        Packed data
  @param     offset     Offset between points
  @param     bracket_pt whether to bracket the point coordinate with (),
                        multipoint need so.
*/

void Geometry::append_points(String *txt, uint32 n_points, wkb_parser *wkb,
                             uint32 offset, bool bracket_pt) const {
  DBUG_ASSERT(0.0 == 0 && 0 == -0 && -0.0 == 0.0);

  while (n_points--) {
    point_xy p;
    wkb->skip_unsafe(offset);
    wkb->scan_xy_unsafe(&p);
    txt->reserve(MAX_DIGITS_IN_DOUBLE * 2 + 1);
    if (bracket_pt) qs_append('(', txt);
    qs_append(p.x, MAX_DIGITS_IN_DOUBLE, txt);
    qs_append(' ', txt);
    qs_append(p.y, MAX_DIGITS_IN_DOUBLE, txt);
    if (bracket_pt) qs_append(')', txt);
    qs_append(',', txt);
  }
}

/**
  Get most bounding rectangle (mbr) for X points

  @param [out] mbr      Result
  @param wkb          Data for point is here.
  @param offset       Offset between points

  @return             false on success, true on error
*/

bool Geometry::get_mbr_for_points(MBR *mbr, wkb_parser *wkb,
                                  uint offset) const {
  uint32 n_points;

  if (wkb->scan_n_points_and_check_data(&n_points, offset)) return true;

  /* Calculate MBR for points */
  while (n_points--) {
    wkb->skip_unsafe(offset);

    point_xy p;
    wkb->scan_xy_unsafe(&p);
    if (!std::isfinite(p.x) || !std::isfinite(p.y)) return true;
    mbr->add_xy(p);
  }
  return false;
}

Geometry::Geometry(const Geometry &geo) {
#if !defined(DBUG_OFF)
  wkbType geotype = geo.get_geotype();
#endif
  DBUG_ASSERT(is_valid_geotype(geotype) &&
              ((geo.get_ptr() != nullptr && geo.get_nbytes() > 0) ||
               (geo.get_ptr() == nullptr && geo.get_nbytes() == 0) ||
               (geo.get_geotype() == wkb_polygon && geo.get_nbytes() == 0)));

  m_ptr = geo.m_ptr;
  m_flags = geo.m_flags;
  m_owner = geo.m_owner;
  set_srid(geo.get_srid());
}

Geometry::~Geometry() {
/*
  Make sure no exceptions can be thrown in destructors of Geometry classes,
  by asserting in debug builds, so that future code won't accidentally throw.

  If an exception is thrown when we destroy a geometry object G,
  although the exception will still be caught and converted to MySQL
  error report, the geometries that are in the same container as G and
  that are placed after G will not be properly destroyed. This is the
  problem we want to address/avoid by forbiding throwing exceptions in
  destructors of Geometry classes.

  Since DBUG_ASSERT only works when DBUG_OFF is not defined, the
  try/catch is only enabled here depending on the same condition, so that
  in release builds we don't have the overhead of the try-catch statement.

  This is true also for destructors of children classes of Geometry.
*/
#if !defined(DBUG_OFF)
  try {
#endif
    if (!is_bg_adapter()) return;

    Geometry::wkbType gt = get_geotype();

    if (gt != Geometry::wkb_polygon) {
      if (get_ownmem() && m_ptr) {
        set_ownmem(false);
        gis_wkb_free(m_ptr);
        m_ptr = nullptr;
      }

      set_nbytes(0);
    }

    donate_data();

#if !defined(DBUG_OFF)
  } catch (...) {
    // Should never throw exceptions in destructor.
    DBUG_ASSERT(false);
  }
#endif
}

/**
  Assignment operator for Geometry class, assignment operators of children
  classes calls this to do general assignment.
  */
Geometry &Geometry::operator=(const Geometry &rhs) {
  if (this == &rhs) return *this;

#if !defined(DBUG_OFF)
  Geometry::wkbType geotype = rhs.get_geotype();
#endif
  DBUG_ASSERT((is_bg_adapter() || rhs.is_bg_adapter()) &&
              m_flags.geotype == rhs.m_flags.geotype &&
              is_valid_geotype(geotype));

  set_bg_adapter(true);

  /*
    Update the mutable state of rhs.
   */
  rhs.set_bg_adapter(true);
  set_srid(rhs.get_srid());
  // Don't set_flags, it's done in operator= of children classes.

  return *this;
}

/***************************** Point *******************************/

Gis_point::Gis_point(const self &pt) : Geometry(pt) {
  size_t nbytes = get_nbytes();
  DBUG_ASSERT((nbytes == SIZEOF_STORED_DOUBLE * GEOM_DIM || nbytes == 0));
  if (nbytes == 0) {
    DBUG_ASSERT(get_ownmem() == false);
    // Allocate even if pt isn't initialized with proper value, this is
    // required behavior from Boost Geometry.
    nbytes = SIZEOF_STORED_DOUBLE * GEOM_DIM;
    set_nbytes(nbytes);
  }

  m_ptr = gis_wkb_fixed_alloc(nbytes);
  if (m_ptr == nullptr) {
    set_nbytes(0);
    set_ownmem(false);
    return;
  }

  if (pt.get_nbytes() > 0)
    memcpy(m_ptr, pt.get_ptr(), pt.get_nbytes());
  else
    memset(m_ptr, 0, nbytes);
  set_ownmem(true);
}

/**
  Deep assignment from point 'p' to this object.
  @param rhs the Gis_point to duplicate from.
*/
Gis_point &Gis_point::operator=(const Gis_point &rhs) {
  if (this == &rhs) return *this;
  Geometry::operator=(rhs);

  // This point may or may not have own memory. we allow this because in bg,
  // std::reverse is called to reverse a linestring/ring, and also,
  // points are of equal size. Not allowed on any other type of geometries.
  DBUG_ASSERT(
      (m_ptr != nullptr && get_nbytes() == SIZEOF_STORED_DOUBLE * GEOM_DIM) ||
      (m_ptr == nullptr && get_nbytes() == 0 && !get_ownmem()));

  DBUG_ASSERT(
      (rhs.get_ptr() != nullptr &&
       rhs.get_nbytes() == SIZEOF_STORED_DOUBLE * GEOM_DIM) ||
      (rhs.get_ptr() == nullptr && rhs.get_nbytes() == 0 && !rhs.get_ownmem()));

  if (m_owner == nullptr) m_owner = rhs.get_owner();

  size_t plen = rhs.get_nbytes();

  // May or may not have own memory. We allow assignment to a Gis_point
  // owning no memory because in BG, std::reverse is called to reverse a
  // linestring/ring, and also, points are of equal size.
  // Not allowed on any other type of geometries.
  if (m_ptr == nullptr) {
    set_nbytes(SIZEOF_STORED_DOUBLE * GEOM_DIM);
    set_ownmem(true);
    m_ptr = gis_wkb_fixed_alloc(get_nbytes());
    if (m_ptr == nullptr) {
      set_nbytes(0);
      set_ownmem(false);
      return *this;
    }
  }

  /*
    Boost Geometry may use a point that is only default constructed that has
    no meaningful value, and in such a case the default value are all zeros.
   */
  if (plen > 0)
    memcpy(m_ptr, rhs.get_ptr(), plen);
  else
    memset(m_ptr, 0, get_nbytes());

  return *this;
}

/**
  Shallow assignment, let this point object refer to the specified memory
  address as its WKB data, and this object never owns the memory assigned.
  @param ptr WKB data address for the point.
  @param len WKB data length for the point.
 */
void Gis_point::set_ptr(void *ptr, size_t len) {
  set_bg_adapter(true);
  if (m_ptr && get_ownmem()) {
    DBUG_ASSERT(get_nbytes() == SIZEOF_STORED_DOUBLE * GEOM_DIM);
    gis_wkb_free(m_ptr);
  }
  m_ptr = ptr;
  set_nbytes(len);
  set_ownmem(false);
  DBUG_ASSERT(
      (m_ptr != nullptr && get_nbytes() == SIZEOF_STORED_DOUBLE * GEOM_DIM) ||
      (m_ptr == nullptr && get_nbytes() == 0));
}

uint32 Gis_point::get_data_size() const {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr))
    return GET_SIZE_ERROR;
  if (get_nbytes() != POINT_DATA_SIZE) return GET_SIZE_ERROR;

  return POINT_DATA_SIZE;
}

bool Gis_point::init_from_wkt(Gis_read_stream *trs, String *wkb,
                              const bool parens) {
  if (check_stack_overrun(trs->thd(), STACK_MIN_SIZE, nullptr)) return true;

  double x, y;
  if ((parens && trs->check_next_symbol('(')) || trs->get_next_number(&x) ||
      trs->get_next_number(&y) || wkb->reserve(POINT_DATA_SIZE, 256) ||
      (parens && trs->check_next_symbol(')')))
    return true;
  q_append(x, wkb);
  q_append(y, wkb);
  return false;
}

/*
  Initialize point using WKB data, the WKB data may come from user input or
  internally table-stored geometry, and user's WKB data may be either endian.
  The WKB data may be of either little endian or big endian, thus we need to
  read them using endian-ness aware functions wkb_get_uint, wkb_get_double,
  and store them as portable (little endian) format into res.
  Only xxx_from_wkb SQL functions can see big endian WKB data, all other
  function Items see portable little endian WKB data.
  This is true for all the init_from_wkb functions of all Geometry classes.
 */
uint Gis_point::init_from_wkb(THD *thd, const char *wkb, uint len,
                              wkbByteOrder bo, String *res) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr)) return 0;
  double x, y;
  if (len < POINT_DATA_SIZE || res->reserve(POINT_DATA_SIZE, 256)) return 0;
  x = wkb_get_double(wkb, bo);
  y = wkb_get_double(wkb + SIZEOF_STORED_DOUBLE, bo);
  q_append(x, res);
  q_append(y, res);
  return POINT_DATA_SIZE;
}

bool Gis_point::get_data_as_wkt(String *txt, wkb_parser *wkb) const {
  point_xy p;
  if (wkb->scan_xy(&p)) return true;
  if (txt->reserve(MAX_DIGITS_IN_DOUBLE * 2 + 3)) return true;
  if (!std::isfinite(p.x) || !std::isfinite(p.y)) return true;
  qs_append('(', txt);
  qs_append(p.x, MAX_DIGITS_IN_DOUBLE, txt);
  qs_append(' ', txt);
  qs_append(p.y, MAX_DIGITS_IN_DOUBLE, txt);
  qs_append(')', txt);
  return false;
}

bool Gis_point::get_mbr(MBR *mbr, wkb_parser *wkb) const {
  point_xy p;
  if (wkb->scan_xy(&p)) return true;
  if (!std::isfinite(p.x) || !std::isfinite(p.y)) return true;
  mbr->add_xy(p);
  return false;
}

bool Gis_point::reverse_coordinates() {
  double x;
  double y;

  if (get_x(&x) || get_y(&y)) {
    return true;
  }

  float8store(get_cptr(), y);
  float8store((get_cptr() + SIZEOF_STORED_DOUBLE), x);

  return false;
}

bool Gis_point::validate_coordinate_range(double srs_angular_unit,
                                          bool *long_out_of_range,
                                          bool *lat_out_of_range,
                                          double *out_of_range_value) {
  double x;
  double y;

  *long_out_of_range = false;
  *lat_out_of_range = false;

  if (get_x(&x) || get_y(&y)) {
    return true; /* purecov: inspected */
  }

  return check_coordinate_range(x, y, srs_angular_unit, long_out_of_range,
                                lat_out_of_range, out_of_range_value);
}

const Geometry::Class_info *Gis_point::get_class_info() const {
  return &point_class;
}

/***************************** LineString *******************************/
uint32 Gis_line_string::get_data_size() const {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr))
    return GET_SIZE_ERROR;

  if (is_length_verified()) return static_cast<uint32>(get_nbytes());

  uint32 n_points;
  uint32 len;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  if (wkb.scan_n_points_and_check_data(&n_points)) return GET_SIZE_ERROR;

  len = 4 + n_points * POINT_DATA_SIZE;
  if (len != get_nbytes()) set_nbytes(len);
  set_length_verified(true);
  return len;
}

// Helper function to get coordinate value from a linestring of WKB format.
inline double coord_val(const char *p, int i, int x) {
  return float8get(p + i * POINT_DATA_SIZE + (x ? SIZEOF_STORED_DOUBLE : 0));
}

bool Gis_line_string::init_from_wkt(Gis_read_stream *trs, String *wkb) {
  if (check_stack_overrun(trs->thd(), STACK_MIN_SIZE, nullptr)) return true;

  uint32 n_points = 0;
  uint32 np_pos = wkb->length();
  Gis_point p(false);

  if (trs->check_next_symbol('(')) return true;
  if (wkb->reserve(4, 512)) return true;
  wkb->length(wkb->length() + 4);  // Reserve space for points

  for (;;) {
    if (p.init_from_wkt(trs, wkb, false)) return true;
    n_points++;
    if (trs->skip_char(','))  // Didn't find ','
      break;
  }

  if (n_points < 2) {
    trs->set_error_msg("Too few points in LINESTRING");
    return true;
  }

  const char *firstpt = nullptr, *lastpt = nullptr;
  if (!is_polygon_ring()) goto out;

  // Make sure all rings of a polygon are closed, and a ring must have
  // at least 4 points.
  firstpt = wkb->ptr() + np_pos + 4;
  lastpt = wkb->ptr() + wkb->length() - POINT_DATA_SIZE;

  if (n_points < 4 || memcmp(lastpt, firstpt, POINT_DATA_SIZE)) return true;

  DBUG_ASSERT(n_points == (lastpt - firstpt) / POINT_DATA_SIZE + 1);

out:

  write_at_position(np_pos, n_points, wkb);
  if (trs->check_next_symbol(')')) return true;
  return false;
}

uint Gis_line_string::init_from_wkb(THD *thd, const char *wkb, uint len,
                                    wkbByteOrder bo, String *res) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr)) return 0;

  uint32 n_points, proper_length;
  const char *wkb_end;
  Gis_point p(false);

  if (len < 4 || (n_points = wkb_get_uint(wkb, bo)) < 2 ||
      (is_polygon_ring() && n_points < 4) || n_points > max_n_points)
    return 0;
  proper_length = 4 + n_points * POINT_DATA_SIZE;
  wkb_end = wkb + proper_length;

  if (len < proper_length) return 0;

  // Make sure all rings of a polygon are closed.
  if (is_polygon_ring() &&
      memcmp(wkb + 4, wkb_end - POINT_DATA_SIZE, POINT_DATA_SIZE))
    return 0;

  if (res->reserve(proper_length, 512)) return 0;

  q_append(n_points, res);
  for (wkb += 4; wkb < wkb_end; wkb += POINT_DATA_SIZE) {
    if (!p.init_from_wkb(thd, wkb, POINT_DATA_SIZE, bo, res)) return 0;
  }

  return proper_length;
}

bool Gis_line_string::get_data_as_wkt(String *txt, wkb_parser *wkb) const {
  uint32 n_points;
  if (wkb->scan_n_points_and_check_data(&n_points) ||
      txt->reserve(1 + ((MAX_DIGITS_IN_DOUBLE + 1) * 2 + 1) * n_points))
    return true;

  qs_append('(', txt);
  while (n_points--) {
    point_xy p;
    wkb->scan_xy_unsafe(&p);
    if (!std::isfinite(p.x) || !std::isfinite(p.y)) return true;
    qs_append(p.x, MAX_DIGITS_IN_DOUBLE, txt);
    qs_append(' ', txt);
    qs_append(p.y, MAX_DIGITS_IN_DOUBLE, txt);
    qs_append(',', txt);
  }
  txt->length(txt->length() - 1);  // Remove end ','
  qs_append(')', txt);
  return false;
}

bool Gis_line_string::get_mbr(MBR *mbr, wkb_parser *wkb) const {
  return get_mbr_for_points(mbr, wkb, 0);
}

int Gis_line_string::geom_length(double *len) const {
  uint32 n_points;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  *len = 0;  // In case of errors
  if (wkb.scan_n_points_and_check_data(&n_points)) return 1;

  point_xy prev;
  wkb.scan_xy_unsafe(&prev);
  while (--n_points) {
    point_xy p;
    wkb.scan_xy_unsafe(&p);
    *len += prev.distance(p);
    if (!std::isfinite(*len)) return 1;
    prev = p;
  }
  return 0;
}

int Gis_line_string::is_closed(int *closed) const {
  uint32 n_points;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (wkb.scan_n_points_and_check_data(&n_points)) return 1;

  if (n_points == 1) {
    *closed = 1;
    return 0;
  }

  point_xy p1, p2;

  /* Get first point. */
  wkb.scan_xy_unsafe(&p1);

  /* Get last point. */
  wkb.skip_unsafe((n_points - 2) * POINT_DATA_SIZE);
  wkb.scan_xy_unsafe(&p2);

  *closed = p1.eq(p2);
  return 0;
}

int Gis_line_string::num_points(uint32 *n_points) const {
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  return wkb.scan_uint4(n_points) ? 1 : 0;
}

int Gis_line_string::start_point(String *result) const {
  uint32 n_points;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  if (wkb.scan_n_points_and_check_data(&n_points)) return 1;
  return create_point(result, &wkb);
}

int Gis_line_string::end_point(String *result) const {
  uint32 n_points;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  if (wkb.scan_n_points_and_check_data(&n_points)) return 1;
  wkb.skip_unsafe((n_points - 1) * POINT_DATA_SIZE);
  return create_point(result, &wkb);
}

int Gis_line_string::point_n(uint32 num, String *result) const {
  uint32 n_points;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  if (num < 1 || wkb.scan_n_points_and_check_data(&n_points) || num > n_points)
    return 1;
  wkb.skip_unsafe((num - 1) * POINT_DATA_SIZE);
  return create_point(result, &wkb);
}

bool Gis_line_string::reverse_coordinates() {
  uint32 num_of_points;

  if (num_points(&num_of_points)) {
    return true;
  }

  for (uint32 i = 0; i < num_of_points; i++) {
    // +4 in below functions to skip numPoints field.
    double x = float8get(get_cptr() + 4 + i * POINT_DATA_SIZE);
    double y =
        float8get(get_cptr() + 4 + i * POINT_DATA_SIZE + SIZEOF_STORED_DOUBLE);

    float8store(get_cptr() + 4 + i * POINT_DATA_SIZE, y);
    float8store(get_cptr() + 4 + i * POINT_DATA_SIZE + SIZEOF_STORED_DOUBLE, x);
  }

  return false;
}

bool Gis_line_string::validate_coordinate_range(double srs_angular_unit,
                                                bool *long_out_of_range,
                                                bool *lat_out_of_range,
                                                double *out_of_range_value) {
  uint32 num_of_points;
  *long_out_of_range = false;
  *lat_out_of_range = false;

  if (num_points(&num_of_points)) {
    return true; /* purecov: inspected */
  }

  for (uint32 i = 0; i < num_of_points; i++) {
    // +4 in below functions to skip numPoints field.
    double x = float8get(get_cptr() + 4 + i * POINT_DATA_SIZE);
    double y =
        float8get(get_cptr() + 4 + i * POINT_DATA_SIZE + SIZEOF_STORED_DOUBLE);

    if (check_coordinate_range(x, y, srs_angular_unit, long_out_of_range,
                               lat_out_of_range, out_of_range_value)) {
      return true;
    }
  }

  return false;
}

const Geometry::Class_info *Gis_line_string::get_class_info() const {
  return &linestring_class;
}

/***************************** Polygon *******************************/
/**
  Copy constructor.
  Coordinate type, closed-ness and direction will never change.
  @param r another polygon of same coordinate type, ring closed-ness and
  ring direction.
*/
Gis_polygon::Gis_polygon(const self &r) : Geometry(r), m_inn_rings(nullptr) {
  Gis_polygon::ring_type *r_out = nullptr, *outer = nullptr;
  Gis_polygon::inner_container_type *r_inners = nullptr, *inners = nullptr;

  if (r.is_bg_adapter() == false || r.get_ptr() == nullptr) return;

  std::unique_ptr<Gis_polygon::ring_type> guard1;
  std::unique_ptr<Gis_polygon::inner_container_type> guard2;

  if (r.get_ptr()) {
    r_out = outer_ring(&r);
    outer = new Gis_polygon::ring_type(*r_out);
    guard1.reset(outer);
    outer->set_owner(this);
    m_ptr = outer;
  }

  if (r.inner_rings()) {
    r_inners = r.inner_rings();
    inners = new Gis_polygon::inner_container_type(*r_inners);
    guard2.reset(inners);
    inners->set_owner(this);
    set_inner_rings(inners);
  }

  set_ownmem(false);
  set_bg_adapter(true);
  guard1.release();
  guard2.release();
}

Gis_polygon::Gis_polygon(const void *wkb, size_t nbytes, const Flags_t &flags,
                         gis::srid_t srid)
    : Geometry(nullptr, nbytes, flags, srid) {
  set_geotype(wkb_polygon);
  // Make use of Gis_wkb_vector::parse_wkb_data.
  inner_container_type v(wkb, nbytes, get_flags(), srid);
  m_ptr = v.get_ptr();
  m_inn_rings = reinterpret_cast<inner_container_type *>(v.get_geo_vect());
  set_ownmem(false);
  if (m_ptr) outer_ring(this)->set_owner(this);
  if (m_inn_rings) m_inn_rings->set_owner(this);

  set_bg_adapter(true);
  v.donate_data();
}

Gis_polygon::~Gis_polygon() {
/* See ~Geometry() for why we do try-catch like this. */
#if !defined(DBUG_OFF)
  try {
#endif
    if (!is_bg_adapter() && !get_ownmem()) return;

    if (m_ptr) {
      if (polygon_is_wkb_form()) {
        if (get_ownmem()) gis_wkb_free(m_ptr);
      } else
        delete outer_ring(this);
      m_ptr = nullptr;
    }
    if (m_inn_rings) {
      delete m_inn_rings;
      m_inn_rings = nullptr;
    }
    /*
      Never need to free polygon's wkb memory here, because if it's one chunk
      given to us, we don't own it; otherwise the two pieces are already freed
      above.
     */
#if !defined(DBUG_OFF)
  } catch (...) {
    // Should never throw exceptions in destructor.
    DBUG_ASSERT(false);
  }
#endif
}

/**
  Deep assignment from polygon 'o' to this object.
  @param rhs the Gis_polygon instance to duplicate from.
*/
Gis_polygon &Gis_polygon::operator=(const Gis_polygon &rhs) {
  if (this == &rhs || !is_bg_adapter() || !rhs.is_bg_adapter()) return *this;
  Geometry::operator=(rhs);

  this->set_flags(rhs.get_flags());
  if (this->m_owner == nullptr) this->m_owner = rhs.get_owner();

  if (m_ptr) delete outer_ring(this);
  if (m_inn_rings) delete m_inn_rings;
  m_ptr = nullptr;
  m_inn_rings = nullptr;

  if (rhs.get_ptr()) {
    Gis_polygon::ring_type *outer =
        new Gis_polygon::ring_type(*outer_ring(&rhs));
    outer->set_owner(this);
    m_ptr = outer;
  }

  if (rhs.inner_rings()) {
    Gis_polygon::inner_container_type *inners =
        new Gis_polygon::inner_container_type(*rhs.inner_rings());
    inners->set_owner(this);
    set_inner_rings(inners);
  }

  return *this;
}

/**
  Set WKB data to this object, the WKB data will be used read only.
  @param ptr WKB data pointer.
  @param len WKB data number of bytes.
 */
void Gis_polygon::set_ptr(void *ptr, size_t len) {
  set_bg_adapter(true);
  ring_type *outer = outer_ring(this);

  if (outer) delete outer;
  if (m_inn_rings) delete m_inn_rings;

  set_nbytes(len);

  Gis_wkb_vector<ring_type> plgn(ptr, len, get_flags(), get_srid());
  m_ptr = plgn.get_ptr();
  m_inn_rings = reinterpret_cast<inner_container_type *>(plgn.get_geo_vect());

  outer = outer_ring(this);
  if (outer != nullptr) outer->set_owner(this);
  if (m_inn_rings != nullptr) m_inn_rings->set_owner(this);
  // Prevent destructor deleting m_ptr/m_inn_rings.
  plgn.donate_data();
}

/**
  Make the polygon's data in a single buffer as WKB format. This polygon
  must be one for BG use before this call, and after this call it can
  never be passed into BG functions directly after this call, and it
  is suitable as a Gis_polygon for MySQL GIS code, because it's exactly the
  same as a Gis_polygon object returned by Geometry::create_from_wkt/wkb.
 */
void Gis_polygon::to_wkb_unparsed() {
  DBUG_ASSERT(polygon_is_wkb_form() == false && is_bg_adapter());

  size_t nbytes = 0;
  void *ptr = get_packed_ptr(this, &nbytes);
  delete outer_ring(this);
  delete m_inn_rings;
  m_ptr = ptr;
  set_nbytes(nbytes);
  m_inn_rings = nullptr;
  polygon_is_wkb_form(true);
  set_bg_adapter(false);
  set_ownmem(true);
}

/**
  Set the specified ring to counter-clockwise(CCW) or clockwise(CW) if it's not.
  Assuems the ring is closed, i.e. the 1st point is the same as the last one.
  Allow duplicate vertices at any position, even rings degraded to a point;
  Works for convex and concave rings; Can detect those with spikes and
  reject them.

  @param want_ccw whether want CCW ring(true) or CW ring(false).
  @return false if successful, true if got error -- invalid geometry data.
 */
bool Gis_polygon_ring::set_ring_order(bool want_ccw) {
  DBUG_ASSERT(is_bg_adapter());
  Gis_polygon_ring &ring = *this;
  double x1, x2, y1, y2, minx = DBL_MAX, miny = DBL_MAX;
  size_t min_i = 0, prev_i, post_i, rsz = ring.size();

  static_assert(sizeof(double) == POINT_DATA_SIZE / 2 &&
                    sizeof(double) == SIZEOF_STORED_DOUBLE,
                "");

  /*
    User input WKT/WKB may contain invalid geometry data that has less
    than 3 points in a polygon ring, so we should return error in this case.
   */
  if (rsz < 3) return true;

  for (size_t i = 0; i < rsz; i++) {
    x1 = ring[i].get<0>();
    y1 = ring[i].get<1>();

    if (i == 0) {
      minx = x1;
      miny = y1;
      continue;
    }

    if (x1 < minx) {
      minx = x1;
      miny = y1;
      min_i = i;
    } else if (x1 == minx) {
      if (y1 < miny) {
        miny = y1;
        min_i = i;
      }
    }
  }

  prev_i = min_i - 1;
  post_i = min_i + 1;

  if (min_i == 0) {
    // 1st pt and last pt is the same pt, i.e. a closed polygon data, we
    // shouldn't use the last pt as prev_i in this case otherwise we will
    // get 0 sign.
    if (ring[0].get<0>() == ring[rsz - 1].get<0>() &&
        ring[0].get<1>() == ring[rsz - 1].get<1>()) {
      prev_i = rsz - 2;
      /*
        Survive from continuous duplicates before prev_i points.
       */
      while (ring[prev_i].get<0>() == ring[min_i].get<0>() &&
             ring[prev_i].get<1>() == ring[min_i].get<1>()) {
        prev_i--;
        /*
          Since the ring must be closed, we will never arrive at the first point
          otherwise the 1st point would be the min_i and all points would be
          the same and this ring would be a point.
         */
        if (prev_i == static_cast<size_t>(-1)) return true;
      }
    } else
      prev_i = rsz - 1;
  } else if (min_i == rsz - 1) {
    // Since we are scanning from 1st point, it's impossible for post_i(0) to
    // be the same as min_i here.
    post_i = 0;
    // Can never come here if all polygon rings are closed.
    return true;
  }

  /*
    Survive from continuous duplicates after min_i points. min_i must be
    the 1st of such duplicates if any, so no duplicates before min_i.
   */
  while (ring[post_i].get<0>() == ring[min_i].get<0>() &&
         ring[post_i].get<1>() == ring[min_i].get<1>()) {
    post_i++;
    /*
      Since the ring must be closed, we will never arrive at the last point
      otherwise the 1st point would be the min_i, i.e. all points are the same,
      and this ring would be a point.
     */
    if (post_i == rsz) return true;
  }

  // The triangle's area tells the direction.
  x1 = ring[min_i].get<0>() - ring[prev_i].get<0>();
  y1 = ring[min_i].get<1>() - ring[prev_i].get<1>();
  x2 = ring[post_i].get<0>() - ring[min_i].get<0>();
  y2 = ring[post_i].get<1>() - ring[min_i].get<1>();
  double sign = x1 * y2 - x2 * y1;

  if (sign == 0) return true;  // Catches error: there is a spike in the ring.

  // Reverse points in the ring, do direct memory manipulation rather
  // than using std::reverse for better performance.
  if ((sign > 0 && !want_ccw) || (sign < 0 && want_ccw)) {
    char *p = static_cast<char *>(ring.get_ptr()) + sizeof(uint32);
    char *q = nullptr, *p0;
    char pt[POINT_DATA_SIZE];
    size_t s = ring.size();

    DBUG_ASSERT(ring.get_nbytes() == (s * POINT_DATA_SIZE + 4));
    p0 = p;

    for (size_t i = 0; i < s / 2; i++, p += POINT_DATA_SIZE) {
      q = p0 + (s - i - 1) * POINT_DATA_SIZE;
      memcpy(&pt, p, POINT_DATA_SIZE);
      memcpy(p, q, POINT_DATA_SIZE);
      memcpy(q, &pt, POINT_DATA_SIZE);
    }
  }
  return false;
}

/**
  Set this polygon's outer ring to be CCW and inner rings to be CW.
  @return on error returns true, on success returns false.
*/
bool Gis_polygon::set_polygon_ring_order() {
  DBUG_ASSERT(is_bg_adapter());
  if (outer().set_ring_order(true /* Ring order: CCW. */)) return true;
  Gis_polygon::inner_container_type::iterator itr;
  Gis_polygon::inner_container_type &inns = inners();
  for (itr = inns.begin(); itr != inns.end(); ++itr)
    if (itr->set_ring_order(false /* Ring order: CW. */)) return true;

  return false;
}

/**
  Make outer ring and inner rings objects for this polygon if it doesn't
  have one yet.
  Outer ring and inner rings have to have separated memory space, because
  we can't predict which one will be edited first. So the polygon object
  doesn't directly have memory, its m_ptr points to the outer ring, its
  m_inn_rings points to the inner rings, each have its own memory address
  and length, and Gis_polygon::get_nbytes returns the sum of them.

  If the polygon doesn't own memory, then there is only one piece of memory
  passed into it and used by it, otherwise the two pieces of memory are
  separately allocated and released.
 */
void Gis_polygon::make_rings() {
  ring_type *outer = nullptr;

  if (this->m_ptr == nullptr) {
    outer = new ring_type(nullptr, 0, Flags_t(wkb_linestring, 0), get_srid());
    outer->set_owner(this);
    this->m_ptr = outer;
  }

  if (m_inn_rings == nullptr) {
    m_inn_rings = new inner_container_type(
        nullptr, 0, Flags_t(wkb_polygon_inner_rings, 0), get_srid());
    m_inn_rings->set_owner(this);
  }
  this->set_ownmem(false);
}

uint32 Gis_polygon::get_data_size() const {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr))
    return GET_SIZE_ERROR;

  uint32 n_linear_rings;
  uint32 len;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (is_length_verified()) return get_nbytes();

  /*
    For a BG adapter polygon, its Gis_polygon::m_ptr points to its outer ring
    rather than the WKB buffer, it is the only exception.
   */
  DBUG_ASSERT(polygon_is_wkb_form() || !is_bg_adapter());

  if (wkb.scan_non_zero_uint4(&n_linear_rings)) return GET_SIZE_ERROR;

  while (n_linear_rings--) {
    uint32 n_points;
    if (wkb.scan_n_points_and_check_data(&n_points)) return GET_SIZE_ERROR;
    wkb.skip_unsafe(n_points * POINT_DATA_SIZE);
  }
  len = static_cast<uint32>(wkb.data() - (const char *)get_data_ptr());
  if (len != get_nbytes()) set_nbytes(len);
  set_length_verified(true);
  return len;
}

bool Gis_polygon::init_from_wkt(Gis_read_stream *trs, String *wkb) {
  if (check_stack_overrun(trs->thd(), STACK_MIN_SIZE, nullptr)) return true;

  uint32 n_linear_rings = 0;
  uint32 lr_pos = wkb->length();

  if (trs->check_next_symbol('(')) return true;
  if (wkb->reserve(4, 512)) return true;
  wkb->length(wkb->length() + 4);  // Reserve space for points

  bool is_first = true;
  for (;;) {
    Gis_line_string ls(false);
    ls.set_props(is_first ? POLYGON_OUTER_RING : POLYGON_INNER_RING);
    is_first = false;

    if (ls.init_from_wkt(trs, wkb)) return true;

    n_linear_rings++;
    if (trs->skip_char(','))  // Didn't find ','
      break;
  }
  write_at_position(lr_pos, n_linear_rings, wkb);
  if (trs->check_next_symbol(')')) return true;
  return false;
}

uint Gis_polygon::init_from_wkb(THD *thd, const char *wkb, uint len,
                                wkbByteOrder bo, String *res) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr)) return 0;

  uint32 n_linear_rings;
  const char *wkb_orig = wkb;

  if (len < 4) return 0;

  if (0 == (n_linear_rings = wkb_get_uint(wkb, bo)) || res->reserve(4, 512))
    return 0;
  wkb += 4;
  len -= 4;
  q_append(n_linear_rings, res);

  bool is_first = true;
  while (n_linear_rings--) {
    Gis_line_string ls(false);
    ls.set_props(is_first ? POLYGON_OUTER_RING : POLYGON_INNER_RING);
    is_first = false;

    uint ls_len = 0;

    if (!(ls_len = ls.init_from_wkb(thd, wkb, len, bo, res))) return 0;

    wkb += ls_len;
    DBUG_ASSERT(len >= ls_len);
    len -= ls_len;
  }

  return (uint)(wkb - wkb_orig);
}

bool Gis_polygon::get_data_as_wkt(String *txt, wkb_parser *wkb) const {
  uint32 n_linear_rings;

  if (wkb->scan_non_zero_uint4(&n_linear_rings)) return true;

  txt->append('(');
  while (n_linear_rings--) {
    uint32 n_points;
    if (wkb->scan_n_points_and_check_data(&n_points) ||
        txt->reserve(2 + ((MAX_DIGITS_IN_DOUBLE + 1) * 2 + 1) * n_points))
      return true;
    qs_append('(', txt);
    append_points(txt, n_points, wkb, 0);
    (*txt)[txt->length() - 1] = ')';  // Replace end ','
    qs_append(',', txt);
  }
  txt->length(txt->length() - 1);  // Remove end ','
  qs_append(')', txt);
  return false;
}

bool Gis_polygon::get_mbr(MBR *mbr, wkb_parser *wkb) const {
  uint32 n_linear_rings;

  if (wkb->scan_non_zero_uint4(&n_linear_rings)) return true;

  while (n_linear_rings--) {
    if (get_mbr_for_points(mbr, wkb, 0)) return true;
  }
  return false;
}

int Gis_polygon::exterior_ring(String *result) const {
  uint32 n_points, n_linear_rings, length;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (wkb.scan_non_zero_uint4(&n_linear_rings) ||
      wkb.scan_n_points_and_check_data(&n_points))
    return 1;
  length = n_points * POINT_DATA_SIZE;
  if (result->reserve(1 + 4 + 4 + length, 512)) return 1;

  q_append((char)wkb_ndr, result);
  q_append((uint32)wkb_linestring, result);
  q_append(n_points, result);
  q_append(wkb.data(), length, result);
  return 0;
}

int Gis_polygon::num_interior_ring(uint32 *n_int_rings) const {
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  if (wkb.scan_non_zero_uint4(n_int_rings)) return 1;
  *n_int_rings -= 1;
  return 0;
}

int Gis_polygon::interior_ring_n(uint32 num, String *result) const {
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  uint32 n_linear_rings;
  uint32 n_points;
  uint32 points_size;

  if (num < 1 || wkb.scan_non_zero_uint4(&n_linear_rings) ||
      num >= n_linear_rings)
    return 1;

  while (num--) {
    if (wkb.scan_n_points_and_check_data(&n_points)) return 1;
    wkb.skip_unsafe(n_points * POINT_DATA_SIZE);
  }
  if (wkb.scan_n_points_and_check_data(&n_points)) return 1;
  points_size = n_points * POINT_DATA_SIZE;
  if (result->reserve(1 + 4 + 4 + points_size, 512)) return 1;

  q_append((char)wkb_ndr, result);
  q_append((uint32)wkb_linestring, result);
  q_append(n_points, result);
  q_append(wkb.data(), points_size, result);
  return 0;
}

bool Gis_polygon::reverse_coordinates() {
  uint32 current_data_offset = 0;
  uint32 numrings;

  if (num_interior_ring(&numrings)) {
    return true;
  }

  numrings += 1;             // add exterior ring to number of rings.
  current_data_offset += 4;  // add numRings header size to data offset.

  for (uint32 i = 0; i < numrings; i++) {
    uint32 num_of_points = uint4korr(get_ucptr() + current_data_offset);
    current_data_offset += 4;  // add linear ring header size to data offset.

    for (uint32 j = 0; j < num_of_points; j++) {
      double x = float8get(get_cptr() + current_data_offset);
      double y =
          float8get(get_cptr() + current_data_offset + SIZEOF_STORED_DOUBLE);

      float8store(get_cptr() + current_data_offset, y);
      float8store(get_cptr() + current_data_offset + SIZEOF_STORED_DOUBLE, x);

      current_data_offset += POINT_DATA_SIZE;
    }
  }

  return false;
}

bool Gis_polygon::validate_coordinate_range(double srs_angular_unit,
                                            bool *long_out_of_range,
                                            bool *lat_out_of_range,
                                            double *out_of_range_value) {
  uint32 numrings;
  *long_out_of_range = false;
  *lat_out_of_range = false;

  if (num_interior_ring(&numrings)) {
    return true; /* purecov: inspected */
  }

  numrings += 1;                   // Add exterior ring to number of rings.
  uint32 current_data_offset = 4;  // Add numRings header size to data offset.

  for (uint32 i = 0; i < numrings; i++) {
    uint32 num_of_points = uint4korr(get_ucptr() + current_data_offset);
    current_data_offset += 4;  // Add linear ring header size to data offset.

    for (uint32 j = 0; j < num_of_points; j++) {
      double x = float8get(get_cptr() + current_data_offset);
      double y =
          float8get(get_cptr() + current_data_offset + SIZEOF_STORED_DOUBLE);

      if (check_coordinate_range(x, y, srs_angular_unit, long_out_of_range,
                                 lat_out_of_range, out_of_range_value)) {
        return true;
      }

      current_data_offset += POINT_DATA_SIZE;
    }
  }

  return false;
}

const Geometry::Class_info *Gis_polygon::get_class_info() const {
  return &polygon_class;
}

/**
   Packup a polygon's outer ring and inner rings into a single chunk of
   memory as result. nbytes returns the number of bytes in WKB data.
   The returned WKB has no WKB header.
   Never call get_ptr to obtain a polygon's WKB data.

   @param geo0 The polygon whose WKB data we want to pack up.
   @param[out] pnbytes Takes back the number of bytes of the packed WKB string.
   @return The address of the packed WKB string buffer.
  */
void *get_packed_ptr(const Geometry *geo0, size_t *pnbytes) {
  DBUG_ASSERT(geo0->get_geotype() == Geometry::wkb_polygon &&
              pnbytes != nullptr);
  const Gis_polygon *geo = static_cast<const Gis_polygon *>(geo0);
  Gis_polygon::ring_type *out_ring = outer_ring(geo);
  Gis_polygon::inner_container_type *inn_rings = geo->inner_rings();
  size_t &nbytes = *pnbytes;

  if (out_ring == nullptr) {
    DBUG_ASSERT(inn_rings == nullptr);
    *pnbytes = 0;
    return nullptr;
  }

  // Inner rings may have out of line rings.
  if (inn_rings) inn_rings->reassemble();

  size_t vallen = sizeof(uint32) + out_ring->get_nbytes() +
                  (inn_rings ? inn_rings->get_nbytes() : 0);
  void *src_val = gis_wkb_alloc(vallen);
  if (src_val == nullptr) {
    nbytes = 0;
    return nullptr;
  }

  memcpy(static_cast<char *>(src_val) + sizeof(uint32), out_ring->get_ptr(),
         out_ring->get_nbytes());

  size_t n_inns = 0;
  if (inn_rings && inn_rings->get_nbytes()) {
    memcpy(
        static_cast<char *>(src_val) + sizeof(uint32) + out_ring->get_nbytes(),
        inn_rings->get_ptr(), inn_rings->get_nbytes());
    n_inns = inn_rings->size();
  }

  DBUG_ASSERT(1 + n_inns <= 0xFFFFFFFF);
  int4store(static_cast<uchar *>(src_val), static_cast<uint32>(1 + n_inns));

  nbytes = vallen;
  return src_val;
}

/**
  Get a polygon's WKB string's starting address. The polygon is already
  packed so that its outer ring and inner rings point to different locations
  of a continuous chunk of WKB buffer.

  @param geo0 The already packed polygon, we want to get its data address.
  @return The WKB string starting address, right after the WKB header if any.
 */
const char *get_packed_ptr(Geometry *geo0) {
  DBUG_ASSERT(geo0->get_geotype() == Geometry::wkb_polygon);
  Gis_polygon *geo = static_cast<Gis_polygon *>(geo0);
  Gis_polygon::ring_type *out_ring = outer_ring(geo);
  Gis_polygon::inner_container_type *inn_rings = geo->inner_rings();
  if (inn_rings)
    DBUG_ASSERT(out_ring->get_cptr() + out_ring->get_nbytes() ==
                inn_rings->get_cptr());
  return (out_ring->get_cptr() - sizeof(uint32) /*polygon's ring count */);
}

/**
  Check whether plgn is packed into its owner mplgn's WKB buffer.
  @param plgn the polygon to be checked
  @param mplgn the multipolygon, owner/holder of plgn.
  @return true if plgn is packed into mplgn, false otherwise.
 */
bool polygon_is_packed(Geometry *plgn, Geometry *mplgn) {
  DBUG_ASSERT(plgn->get_geotype() == Geometry::wkb_polygon &&
              mplgn->get_geotype() == Geometry::wkb_multipolygon);
  Gis_polygon *geo = static_cast<Gis_polygon *>(plgn);
  Gis_polygon::ring_type *out_ring = outer_ring(geo);
  Gis_polygon::inner_container_type *inn_rings = geo->inner_rings();
  const char *orstart = out_ring->get_cptr();
  bool ret = false;

  if (orstart < mplgn->get_cptr() + mplgn->get_nbytes() &&
      orstart > mplgn->get_cptr()) {
    // This polygon is already stored packed and inline
    if (inn_rings && inn_rings->get_nbytes())
      DBUG_ASSERT(orstart + out_ring->get_nbytes() == inn_rings->get_ptr());

    ret = true;
  }

  return ret;
}

void own_rings(Geometry *geo0) {
  DBUG_ASSERT(geo0->get_geotype() == Geometry::wkb_polygon);
  Gis_polygon *geo = static_cast<Gis_polygon *>(geo0);

  if (outer_ring(geo)) outer_ring(geo)->set_owner(geo);
  if (geo->inner_rings()) geo->inner_rings()->set_owner(geo);
}

/***************************** MultiPoint *******************************/
uint32 Gis_multi_point::get_data_size() const {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr))
    return GET_SIZE_ERROR;

  uint32 n_points;
  uint32 len;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (is_length_verified()) return get_nbytes();
  if (wkb.scan_n_points_and_check_data(&n_points, WKB_HEADER_SIZE))
    return GET_SIZE_ERROR;

  len = 4 + n_points * (POINT_DATA_SIZE + WKB_HEADER_SIZE);
  if (len != get_nbytes()) set_nbytes(len);
  set_length_verified(true);
  return len;
}

bool Gis_multi_point::init_from_wkt(Gis_read_stream *trs, String *wkb) {
  if (check_stack_overrun(trs->thd(), STACK_MIN_SIZE, nullptr)) return true;

  uint32 n_points = 0;
  uint32 np_pos = wkb->length();
  Gis_point p(false);

  if (trs->check_next_symbol('(')) return true;
  if (wkb->reserve(4, 512)) return true;
  wkb->length(wkb->length() + 4);  // Reserve space for points

  /*
    According to OGC, the WKT for a multipoint is something like:
    MULTIPOINT((1 1), (2 2))
    and to be backward compatible with older versions of MySQL, we still
    support the MySQL format:
    MULTIPOINT(1 1, 2 2)
    But we don't allow the mixture of both formats in the same multipoint
    geometry.
  */
  const bool match_pt_lbra =
      (trs->get_next_toc_type() == Gis_read_stream::l_bra);

  for (;;) {
    if (wkb->reserve(1 + 4, 512)) return true;
    q_append((char)wkb_ndr, wkb);
    q_append((uint32)wkb_point, wkb);

    if (match_pt_lbra && trs->check_next_symbol('(')) return true;

    if (p.init_from_wkt(trs, wkb, false)) return true;

    if (match_pt_lbra && trs->check_next_symbol(')')) return true;

    n_points++;
    if (trs->skip_char(','))  // Didn't find ','
      break;
  }
  write_at_position(np_pos, n_points, wkb);  // Store number of found points
  if (trs->check_next_symbol(')')) return true;
  return false;
}

uint Gis_multi_point::init_from_wkb(THD *thd, const char *wkb, uint len,
                                    wkbByteOrder bo, String *res) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr)) return 0;

  uint32 n_points;
  uint proper_size;
  Gis_point p(false);
  const char *wkb_end;

  if (len < 4 || (n_points = wkb_get_uint(wkb, bo)) > max_n_points) return 0;
  proper_size = 4 + n_points * (WKB_HEADER_SIZE + POINT_DATA_SIZE);

  if (len < proper_size || res->reserve(proper_size, 512)) return 0;

  q_append(n_points, res);
  wkb_end = wkb + proper_size;
  for (wkb += 4; wkb < wkb_end; wkb += (WKB_HEADER_SIZE + POINT_DATA_SIZE)) {
    write_wkb_header(res, wkb_point);
    if ((*wkb != wkb_xdr && *wkb != wkb_ndr) ||
        wkb_point != uint4korr(wkb + 1) ||
        !p.init_from_wkb(thd, wkb + WKB_HEADER_SIZE, POINT_DATA_SIZE,
                         (wkbByteOrder)wkb[0], res))
      return 0;
  }
  return proper_size;
}

bool Gis_multi_point::get_data_as_wkt(String *txt, wkb_parser *wkb) const {
  uint32 n_points;

  txt->append('(');
  if (wkb->scan_n_points_and_check_data(&n_points, WKB_HEADER_SIZE) ||
      txt->reserve(((MAX_DIGITS_IN_DOUBLE + 1) * 2 + 1) * n_points))
    return true;

  /*
    Now we will output multipoint in OGC format, i.e. for each of its point,
    bracket its coordinates with ().
  */
  append_points(txt, n_points, wkb, WKB_HEADER_SIZE, true);
  txt->length(txt->length() - 1);  // Remove end ','
  qs_append(')', txt);
  return false;
}

bool Gis_multi_point::get_mbr(MBR *mbr, wkb_parser *wkb) const {
  return get_mbr_for_points(mbr, wkb, WKB_HEADER_SIZE);
}

int Gis_multi_point::num_geometries(uint32 *num) const {
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  return wkb.scan_non_zero_uint4(num) ? 1 : 0;
}

int Gis_multi_point::geometry_n(uint32 num, String *result) const {
  uint32 n_points;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (num < 1 || wkb.scan_n_points_and_check_data(&n_points, WKB_HEADER_SIZE) ||
      num > n_points || result->reserve(WKB_HEADER_SIZE + POINT_DATA_SIZE, 32))
    return 1;
  wkb.skip_unsafe((num - 1) * (WKB_HEADER_SIZE + POINT_DATA_SIZE));

  q_append(wkb.data(), WKB_HEADER_SIZE + POINT_DATA_SIZE, result);
  return 0;
}

bool Gis_multi_point::reverse_coordinates() {
  uint32 current_data_offset = 0;
  uint32 num_of_points;
  if (num_geometries(&num_of_points)) {
    return true;
  }

  current_data_offset += 4;  // add number of points header to offset.

  for (uint32 i = 0; i < num_of_points; i++) {
    current_data_offset +=
        WKB_HEADER_SIZE;  // since each point includes a header.

    double x = float8get(get_cptr() + current_data_offset);
    double y =
        float8get(get_cptr() + current_data_offset + SIZEOF_STORED_DOUBLE);

    float8store(get_cptr() + current_data_offset, y);
    float8store(get_cptr() + current_data_offset + SIZEOF_STORED_DOUBLE, x);

    current_data_offset += POINT_DATA_SIZE;
  }

  return false;
}

bool Gis_multi_point::validate_coordinate_range(double srs_angular_unit,
                                                bool *long_out_of_range,
                                                bool *lat_out_of_range,
                                                double *out_of_range_value) {
  uint32 num_of_points;
  *long_out_of_range = false;
  *lat_out_of_range = false;

  if (num_geometries(&num_of_points)) {
    return true; /* purecov: inspected */
  }

  uint32 current_data_offset = 4;  // Add number of points header to offset.

  for (uint32 i = 0; i < num_of_points; i++) {
    current_data_offset +=
        WKB_HEADER_SIZE;  // Since each point includes a header.

    double x = float8get(get_cptr() + current_data_offset);
    double y =
        float8get(get_cptr() + current_data_offset + SIZEOF_STORED_DOUBLE);

    if (check_coordinate_range(x, y, srs_angular_unit, long_out_of_range,
                               lat_out_of_range, out_of_range_value)) {
      return true;
    }

    current_data_offset += POINT_DATA_SIZE;
  }

  return false;
}

const Geometry::Class_info *Gis_multi_point::get_class_info() const {
  return &multipoint_class;
}

/***************************** MultiLineString *******************************/
uint32 Gis_multi_line_string::get_data_size() const {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr))
    return GET_SIZE_ERROR;

  uint32 n_line_strings;
  uint32 len;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (is_length_verified()) return get_nbytes();
  if (wkb.scan_non_zero_uint4(&n_line_strings)) return GET_SIZE_ERROR;

  while (n_line_strings--) {
    uint32 n_points;

    if (wkb.skip_wkb_header() || wkb.scan_n_points_and_check_data(&n_points))
      return GET_SIZE_ERROR;

    wkb.skip_unsafe(n_points * POINT_DATA_SIZE);
  }

  len = static_cast<uint32>(wkb.data() - (const char *)get_data_ptr());
  if (len != get_nbytes()) set_nbytes(len);
  set_length_verified(true);
  return len;
}

bool Gis_multi_line_string::init_from_wkt(Gis_read_stream *trs, String *wkb) {
  if (check_stack_overrun(trs->thd(), STACK_MIN_SIZE, nullptr)) return true;

  uint32 n_line_strings = 0;
  uint32 ls_pos = wkb->length();

  if (trs->check_next_symbol('(')) return true;
  if (wkb->reserve(4, 512)) return true;
  wkb->length(wkb->length() + 4);  // Reserve space for points

  for (;;) {
    Gis_line_string ls(false);

    if (wkb->reserve(1 + 4, 512)) return true;
    q_append((char)wkb_ndr, wkb);
    q_append((uint32)wkb_linestring, wkb);

    if (ls.init_from_wkt(trs, wkb)) return true;
    n_line_strings++;
    if (trs->skip_char(','))  // Didn't find ','
      break;
  }
  write_at_position(ls_pos, n_line_strings, wkb);
  if (trs->check_next_symbol(')')) return true;
  return false;
}

uint Gis_multi_line_string::init_from_wkb(THD *thd, const char *wkb, uint len,
                                          wkbByteOrder bo, String *res) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr)) return 0;

  uint32 n_line_strings;
  const char *wkb_orig = wkb;

  if (len < 4 || (n_line_strings = wkb_get_uint(wkb, bo)) < 1) return 0;

  if (res->reserve(4, 512)) return 0;
  q_append(n_line_strings, res);

  wkb += 4;
  len -= 4;

  while (n_line_strings--) {
    Gis_line_string ls(false);
    uint ls_len = 0;

    if ((len < WKB_HEADER_SIZE) || uint4korr(wkb + 1) != wkb_linestring ||
        (*wkb != wkb_xdr && *wkb != wkb_ndr) ||
        res->reserve(WKB_HEADER_SIZE, 512))
      return 0;

    write_wkb_header(res, wkb_linestring);
    if (!(ls_len = ls.init_from_wkb(thd, wkb + WKB_HEADER_SIZE,
                                    len - WKB_HEADER_SIZE, (wkbByteOrder)wkb[0],
                                    res)))
      return 0;
    ls_len += WKB_HEADER_SIZE;
    ;
    wkb += ls_len;
    DBUG_ASSERT(len >= ls_len);
    len -= ls_len;
  }
  return (uint)(wkb - wkb_orig);
}

bool Gis_multi_line_string::get_data_as_wkt(String *txt,
                                            wkb_parser *wkb) const {
  uint32 n_line_strings;

  if (wkb->scan_non_zero_uint4(&n_line_strings)) return true;

  txt->append('(');
  while (n_line_strings--) {
    uint32 n_points;

    if (wkb->skip_wkb_header() ||
        wkb->scan_n_points_and_check_data(&n_points) ||
        txt->reserve(2 + ((MAX_DIGITS_IN_DOUBLE + 1) * 2 + 1) * n_points))
      return true;
    qs_append('(', txt);
    append_points(txt, n_points, wkb, 0);
    (*txt)[txt->length() - 1] = ')';
    qs_append(',', txt);
  }
  txt->length(txt->length() - 1);
  qs_append(')', txt);
  return false;
}

bool Gis_multi_line_string::get_mbr(MBR *mbr, wkb_parser *wkb) const {
  uint32 n_line_strings;

  if (wkb->scan_non_zero_uint4(&n_line_strings)) return true;

  while (n_line_strings--) {
    if (wkb->skip_wkb_header() || get_mbr_for_points(mbr, wkb, 0)) return true;
  }
  return false;
}

int Gis_multi_line_string::num_geometries(uint32 *num) const {
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  return wkb.scan_non_zero_uint4(num) ? 1 : 0;
}

int Gis_multi_line_string::geometry_n(uint32 num, String *result) const {
  uint32 n_line_strings, n_points, length;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (wkb.scan_non_zero_uint4(&n_line_strings)) return 1;

  if ((num > n_line_strings) || (num < 1)) return 1;

  for (;;) {
    if (wkb.skip_wkb_header() || wkb.scan_n_points_and_check_data(&n_points))
      return 1;
    length = POINT_DATA_SIZE * n_points;
    if (!--num) break;
    wkb.skip_unsafe(length);
  }
  return result->append(wkb.data() - 4 - WKB_HEADER_SIZE,
                        length + 4 + WKB_HEADER_SIZE, static_cast<size_t>(0));
}

int Gis_multi_line_string::geom_length(double *len) const {
  uint32 n_line_strings;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (wkb.scan_non_zero_uint4(&n_line_strings)) return 1;

  *len = 0;
  while (n_line_strings--) {
    double ls_len;
    Gis_line_string ls(false);
    if (wkb.skip_wkb_header()) return 1;
    ls.set_data_ptr(&wkb);
    if (ls.geom_length(&ls_len)) return 1;
    *len += ls_len;
    /*
      We know here that ls was ok, so we can call the trivial function
      Gis_line_string::get_data_size without error checking.
    */
    wkb.skip_unsafe(ls.get_data_size());
  }
  return 0;
}

int Gis_multi_line_string::is_closed(int *closed) const {
  uint32 n_line_strings;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (wkb.scan_non_zero_uint4(&n_line_strings)) return 1;

  while (n_line_strings--) {
    Gis_line_string ls(false);
    if (wkb.skip_wkb_header()) return 1;
    ls.set_data_ptr(&wkb);
    if (ls.is_closed(closed)) return 1;
    if (!*closed) return 0;
    wkb.skip_unsafe(ls.get_data_size());
  }
  return 0;
}

bool Gis_multi_line_string::reverse_coordinates() {
  uint32 num_of_linestrings;
  size_t current_data_offset = 4;  // Skip num_wkbLineStrings header size.

  String str(get_cptr(), get_nbytes(), &my_charset_bin);

  if (num_geometries(&num_of_linestrings)) {
    return true;
  }
  for (uint32 i = 1; i <= num_of_linestrings; i++) {
    String result;

    if (geometry_n(i, &result)) {
      return true;
    }

    Geometry *g;
    Geometry_buffer buffer;
    if (!(g = Geometry::construct(&buffer, &result, false))) {
      return true;
    }

    if (g->reverse_coordinates()) {
      return true;
    }

    if (str.replace(current_data_offset, result.length(), result.ptr(),
                    result.length())) {
      return true;
    }

    current_data_offset += result.length();
  }

  return false;
}

bool Gis_multi_line_string::validate_coordinate_range(
    double srs_angular_unit, bool *long_out_of_range, bool *lat_out_of_range,
    double *out_of_range_value) {
  uint32 num_of_linestrings;
  *long_out_of_range = false;
  *lat_out_of_range = false;

  if (num_geometries(&num_of_linestrings)) {
    return true; /* purecov: inspected */
  }

  for (uint32 i = 1; i <= num_of_linestrings; i++) {
    String result;

    if (geometry_n(i, &result)) {
      return true; /* purecov: inspected */
    }

    Geometry *g;
    Geometry_buffer buffer;
    if (!(g = Geometry::construct(&buffer, &result, false))) {
      return true; /* purecov: inspected */
    }

    if (g->validate_coordinate_range(srs_angular_unit, long_out_of_range,
                                     lat_out_of_range, out_of_range_value)) {
      return true;
    }
  }

  return false;
}

const Geometry::Class_info *Gis_multi_line_string::get_class_info() const {
  return &multilinestring_class;
}

/***************************** MultiPolygon *******************************/
uint32 Gis_multi_polygon::get_data_size() const {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr))
    return GET_SIZE_ERROR;

  uint32 n_polygons;
  uint32 len;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());

  if (is_length_verified()) return get_nbytes();
  if (wkb.scan_non_zero_uint4(&n_polygons)) return GET_SIZE_ERROR;

  while (n_polygons--) {
    uint32 n_linear_rings;
    if (wkb.skip_wkb_header() || wkb.scan_non_zero_uint4(&n_linear_rings))
      return GET_SIZE_ERROR;

    while (n_linear_rings--) {
      uint32 n_points;

      if (wkb.scan_n_points_and_check_data(&n_points)) return GET_SIZE_ERROR;

      wkb.skip_unsafe(n_points * POINT_DATA_SIZE);
    }
  }

  len = static_cast<uint32>(wkb.data() - (const char *)get_data_ptr());
  if (len != get_nbytes()) set_nbytes(len);
  set_length_verified(true);
  return len;
}

bool Gis_multi_polygon::init_from_wkt(Gis_read_stream *trs, String *wkb) {
  if (check_stack_overrun(trs->thd(), STACK_MIN_SIZE, nullptr)) return true;

  uint32 n_polygons = 0;
  uint32 np_pos = wkb->length();
  Gis_polygon p(false);

  if (trs->check_next_symbol('(')) return true;
  if (wkb->reserve(4, 512)) return true;
  wkb->length(wkb->length() + 4);  // Reserve space for points

  for (;;) {
    if (wkb->reserve(1 + 4, 512)) return true;
    q_append((char)wkb_ndr, wkb);
    q_append((uint32)wkb_polygon, wkb);

    if (p.init_from_wkt(trs, wkb)) return true;
    n_polygons++;
    if (trs->skip_char(','))  // Didn't find ','
      break;
  }
  write_at_position(np_pos, n_polygons, wkb);
  if (trs->check_next_symbol(')')) return true;
  return false;
}

uint Gis_multi_polygon::init_from_wkb(THD *thd, const char *wkb, uint len,
                                      wkbByteOrder bo, String *res) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr)) return 0;

  uint32 n_poly;
  const char *wkb_orig = wkb;

  if (len < 4) return 0;
  n_poly = wkb_get_uint(wkb, bo);

  if (res->reserve(4, 512)) return 0;
  q_append(n_poly, res);

  wkb += 4;
  len -= 4;

  while (n_poly--) {
    Gis_polygon p(false);
    uint p_len = 0;

    if (len < WKB_HEADER_SIZE || uint4korr(wkb + 1) != wkb_polygon ||
        (*wkb != wkb_xdr && *wkb != wkb_ndr) ||
        res->reserve(WKB_HEADER_SIZE, 512))
      return 0;
    write_wkb_header(res, wkb_polygon);
    if (!(p_len =
              p.init_from_wkb(thd, wkb + WKB_HEADER_SIZE, len - WKB_HEADER_SIZE,
                              (wkbByteOrder)wkb[0], res)))
      return 0;
    p_len += WKB_HEADER_SIZE;
    wkb += p_len;
    DBUG_ASSERT(len >= p_len);
    len -= p_len;
  }
  return (uint)(wkb - wkb_orig);
}

bool Gis_multi_polygon::get_data_as_wkt(String *txt, wkb_parser *wkb) const {
  uint32 n_polygons;

  if (wkb->scan_non_zero_uint4(&n_polygons)) return true;

  txt->append('(');
  while (n_polygons--) {
    uint32 n_linear_rings;

    if (wkb->skip_wkb_header() || wkb->scan_non_zero_uint4(&n_linear_rings) ||
        txt->reserve(1, 512))
      return true;
    q_append('(', txt);

    while (n_linear_rings--) {
      uint32 n_points;
      if (wkb->scan_n_points_and_check_data(&n_points) ||
          txt->reserve(2 + ((MAX_DIGITS_IN_DOUBLE + 1) * 2 + 1) * n_points))
        return true;
      qs_append('(', txt);
      append_points(txt, n_points, wkb, 0);
      (*txt)[txt->length() - 1] = ')';
      qs_append(',', txt);
    }
    (*txt)[txt->length() - 1] = ')';
    qs_append(',', txt);
  }
  txt->length(txt->length() - 1);
  qs_append(')', txt);
  return false;
}

bool Gis_multi_polygon::get_mbr(MBR *mbr, wkb_parser *wkb) const {
  uint32 n_polygons;

  if (wkb->scan_non_zero_uint4(&n_polygons)) return true;

  while (n_polygons--) {
    uint32 n_linear_rings;
    if (wkb->skip_wkb_header() || wkb->scan_non_zero_uint4(&n_linear_rings))
      return true;

    while (n_linear_rings--) {
      if (get_mbr_for_points(mbr, wkb, 0)) return true;
    }
  }
  return false;
}

int Gis_multi_polygon::num_geometries(uint32 *num) const {
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  return wkb.scan_non_zero_uint4(num) ? 1 : 0;
}

int Gis_multi_polygon::geometry_n(uint32 num, String *result) const {
  uint32 n_polygons;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  const char *start_of_polygon = wkb.data();

  if (wkb.scan_non_zero_uint4(&n_polygons)) return 1;

  if (num > n_polygons || num < 1) return -1;

  do {
    uint32 n_linear_rings;
    start_of_polygon = wkb.data();

    if (wkb.skip_wkb_header() || wkb.scan_non_zero_uint4(&n_linear_rings))
      return 1;

    while (n_linear_rings--) {
      uint32 n_points;
      if (wkb.scan_n_points_and_check_data(&n_points)) return 1;
      wkb.skip_unsafe(POINT_DATA_SIZE * n_points);
    }
  } while (--num);
  if (wkb.no_data(0))  // We must check last segment
    return 1;
  return result->append(start_of_polygon,
                        (uint32)(wkb.data() - start_of_polygon),
                        static_cast<size_t>(0));
}

bool Gis_multi_polygon::reverse_coordinates() {
  uint32 num_of_polygons;
  size_t current_data_offset = 4;  // Skip num_polygons header size.

  String str(get_cptr(), get_nbytes(), &my_charset_bin);

  if (num_geometries(&num_of_polygons)) {
    return true;
  }

  for (uint32 i = 1; i <= num_of_polygons; i++) {
    String result;

    if (geometry_n(i, &result)) {
      return true;
    }

    Geometry *g;
    Geometry_buffer buffer;
    if (!(g = Geometry::construct(&buffer, &result, false))) {
      return true;
    }

    if (g->reverse_coordinates()) {
      return true;
    }

    if (str.replace(current_data_offset, result.length(), result.ptr(),
                    result.length())) {
      return true;
    }

    current_data_offset += result.length();
  }

  return false;
}

bool Gis_multi_polygon::validate_coordinate_range(double srs_angular_unit,
                                                  bool *long_out_of_range,
                                                  bool *lat_out_of_range,
                                                  double *out_of_range_value) {
  uint32 num_of_polygons;
  *long_out_of_range = false;
  *lat_out_of_range = false;

  if (num_geometries(&num_of_polygons)) {
    return true; /* purecov: inspected */
  }

  for (uint32 i = 1; i <= num_of_polygons; i++) {
    String result;

    if (geometry_n(i, &result)) {
      return true; /* purecov: inspected */
    }

    Geometry *g;
    Geometry_buffer buffer;
    if (!(g = Geometry::construct(&buffer, &result, false))) {
      return true; /* purecov: inspected */
    }

    if (g->validate_coordinate_range(srs_angular_unit, long_out_of_range,
                                     lat_out_of_range, out_of_range_value)) {
      return true;
    }
  }

  return false;
}

const Geometry::Class_info *Gis_multi_polygon::get_class_info() const {
  return &multipolygon_class;
}

/************************* GeometryCollection ****************************/

/**
  Create a Geometry object from WKB.

  This function creates an intermediate geometry object which may have wrong
  length property (longer than what's needed by the geometry), use it only
  within this class. Do not check for exact length here, caller will do
  that if necessary.

  @param wkb The input WKB.
  @param buffer A buffer for the output geometry.
  @return NULL if the input WKB was found invalid. Otherwise, the constructed
          geometry.
*/
Geometry *Gis_geometry_collection::scan_header_and_create(
    wkb_parser *wkb, Geometry_buffer *buffer) {
  Geometry *geom;
  wkb_header header;

  if (wkb->scan_wkb_header(&header) ||
      !(geom = create_by_typeid(buffer, header.wkb_type)))
    return nullptr;
  geom->set_data_ptr(wkb->data(), wkb->length());

  /*
    The length in geom might be wrong, since it's set to the total length of
    the geometry collection's WKB. Such error is only allowed for temporary
    geometry objects created here.

    Correct the length only for points because other types has known structure
    and can deduce the valid length. But for point we have to always require
    the exact length.
  */
  if (geom->get_type() == wkb_point) {
    if (geom->get_nbytes() < POINT_DATA_SIZE) return nullptr;
    geom->set_nbytes(POINT_DATA_SIZE);
  }

  return geom;
}

/**
  Append geometry into geometry collection which can be empty.
  @param geo geometry to be appended, it can't be empty.
  @param gcbuf this geometry collection's data buffer, it's of GEOMETRY format
         and is a separate String buffer.
  @return false if no error, otherwise true.

 */
bool Gis_geometry_collection::append_geometry(const Geometry *geo,
                                              String *gcbuf) {
  uint32 collection_len = gcbuf->length(), geo_len = geo->get_data_size();
  if (geo_len == GET_SIZE_ERROR) return true;
  DBUG_ASSERT(collection_len == 0 ||
              get_data_size() == collection_len - GEOM_HEADER_SIZE);
  if (gcbuf->reserve((collection_len == 0 ? GEOM_HEADER_SIZE + 4 : 0) +
                         geo_len + WKB_HEADER_SIZE,
                     512))
    return true;

  char *ptr = gcbuf->ptr();
  uint32 extra = 0;
  if (collection_len == 0) {
    collection_len = GEOM_HEADER_SIZE + 4;
    extra = GEOM_HEADER_SIZE;
    write_geometry_header(ptr, geo->get_srid(), wkb_geometrycollection, 0);
    set_srid(geo->get_srid());
    has_geom_header_space(true);
  }

  // Skip GEOMETRY header.
  ptr += GEOM_HEADER_SIZE;
  const char *start = ptr;

  int4store(ptr, uint4korr(ptr) + 1);  // Increment object count.
  ptr += collection_len - GEOM_HEADER_SIZE;
  ptr = write_wkb_header(ptr, geo->get_type());
  memcpy(ptr, geo->get_data_ptr(), geo_len);
  gcbuf->length(collection_len + geo_len + WKB_HEADER_SIZE);
  set_data_ptr(start, extra + collection_len + geo_len - SRID_SIZE);
  return false;
}

/**
  Append geometry into geometry collection, which can be empty. This object
  must be created from default constructor or below one:
  Gis_geometry_collection(gis::srid_t srid, wkbType gtype,
                          const String *gbuf,
                          String *gcbuf);

  @param srid srid of geometry to be appended.
  @param gtype type of geometry to be appended.
  @param gbuf WKB data of geometry to be appended, gbuf->ptr isn't NULL and
         points right after the WKB header, this buffer can't be empty.
  @param gcbuf this geometry collection's data buffer, it's of GEOMETRY format
         and is a separate String buffer.
  @return false if no error, otherwise true.

 */
bool Gis_geometry_collection::append_geometry(gis::srid_t srid, wkbType gtype,
                                              const String *gbuf,
                                              String *gcbuf) {
  DBUG_ASSERT(gbuf != nullptr && gbuf->ptr() != nullptr && gbuf->length() > 0);

  uint32 collection_len = gcbuf->length(), geo_len = gbuf->length();
  DBUG_ASSERT(collection_len == 0 ||
              get_data_size() == collection_len - GEOM_HEADER_SIZE);
  if (gcbuf->reserve((collection_len == 0 ? GEOM_HEADER_SIZE + 4 : 0) +
                         geo_len + WKB_HEADER_SIZE,
                     512))
    return true;

  char *ptr = gcbuf->ptr();
  uint32 extra = 0;
  if (collection_len == 0) {
    collection_len = GEOM_HEADER_SIZE + 4;
    extra = GEOM_HEADER_SIZE;
    write_geometry_header(ptr, srid, wkb_geometrycollection, 0);
    set_srid(srid);
    has_geom_header_space(true);
  } else if (srid != get_srid())
    return true;

  // Skip GEOMETRY header.
  ptr += GEOM_HEADER_SIZE;
  const char *start = ptr;

  int4store(ptr, uint4korr(ptr) + 1);  // Increment object count.
  ptr += collection_len - GEOM_HEADER_SIZE;
  ptr = write_wkb_header(ptr, gtype);
  memcpy(ptr, gbuf->ptr(), geo_len);
  gcbuf->length(collection_len + geo_len + WKB_HEADER_SIZE);
  set_data_ptr(start, extra + collection_len + geo_len - SRID_SIZE);
  return false;
}

/**
  Create a geometry collection from a single geometry, and the created object
  refers to position right after the WKB header inside the 'gcbuf' buffer.
  @param srid the SRID of the first geometry to put into this
              geometry collection. Its SRID is used as the SRID of this
              geometry collection.
  @param gtype the type of the first geometry to put into this object.
  @param gbuf stores the WKB data of the first geometry to put into this object,
              not including its WKB header. if gbuf is NULL or gbuf->ptr is
              NULL, the created geometry collection is empty.
  @param gcbuf this geometry collection's data buffer in GEOMETRY format.
 */
Gis_geometry_collection::Gis_geometry_collection(gis::srid_t srid,
                                                 wkbType gtype,
                                                 const String *gbuf,
                                                 String *gcbuf)
    : Geometry(nullptr, 0, Flags_t(wkb_geometrycollection, 0), srid) {
  uint32 geo_len = gbuf ? gbuf->length() : 0, total_len = 0;
  DBUG_ASSERT(
      (gbuf == nullptr || (gbuf->ptr() == nullptr && gbuf->length() == 0)) ||
      (gbuf->ptr() != nullptr && gbuf->length() > 0));
  total_len = geo_len + sizeof(uint32) /*NUM-objs*/ + SRID_SIZE +
              WKB_HEADER_SIZE + (geo_len > 0 ? WKB_HEADER_SIZE : 0);

  // Reserve 512 bytes extra space for geometries to be appended later,
  // to avoid some reallocations.
  if (gcbuf->reserve(total_len + 512, 1024))
    my_error(ER_OUTOFMEMORY, total_len + 512);

  char *ptr = gcbuf->ptr();
  const char *start = ptr + GEOM_HEADER_SIZE;

  ptr = write_geometry_header(ptr, srid, Geometry::wkb_geometrycollection,
                              geo_len ? 1 : 0);
  if (geo_len > 0) {
    ptr = write_wkb_header(ptr, gtype);
    memcpy(ptr, gbuf->ptr(), geo_len);
  }

  gcbuf->length(total_len);
  set_data_ptr(start, total_len - GEOM_HEADER_SIZE);
  set_srid(srid);
  has_geom_header_space(true);
}

/**
  Create a geometry collection from a single geometry, and this object refer
  to position right after the WKB header inside the 'gcbuf' buffer.
  @param geo the first valid geometry to put into this geometry collection.
         Its SRID is used as the SRID of this geometry collection. It must be
         a valid geometry.
  @param gcbuf this geometry collection's data buffer in GEOMETRY format.
 */
Gis_geometry_collection::Gis_geometry_collection(Geometry *geo, String *gcbuf)
    : Geometry(nullptr, 0, Flags_t(wkb_geometrycollection, 0),
               geo->get_srid()) {
  DBUG_ASSERT(geo != nullptr && geo->get_ptr() != nullptr);
  uint32 geo_len = geo->get_data_size(), total_len = 0;
  DBUG_ASSERT(geo_len != GET_SIZE_ERROR);
  total_len =
      geo_len + sizeof(uint32) /*NUM-objs*/ + SRID_SIZE + WKB_HEADER_SIZE * 2;

  // Reserve 512 bytes extra space for geometries to be appended later,
  // to avoid some reallocations.
  if (gcbuf->reserve(total_len + 512, 1024))
    my_error(ER_OUTOFMEMORY, total_len + 512);

  char *ptr = gcbuf->ptr();
  const char *start = ptr + GEOM_HEADER_SIZE;

  ptr = write_geometry_header(ptr, geo->get_srid(),
                              Geometry::wkb_geometrycollection, 1);
  ptr = write_wkb_header(ptr, geo->get_type());

  memcpy(ptr, geo->get_data_ptr(), geo_len);
  gcbuf->length(total_len);
  set_data_ptr(start, total_len - GEOM_HEADER_SIZE);
  set_srid(geo->get_srid());
  has_geom_header_space(true);
}

uint32 Gis_geometry_collection::get_data_size() const {
  if (check_stack_overrun(current_thd, STACK_MIN_SIZE, nullptr))
    return GET_SIZE_ERROR;

  uint32 n_objects = 0;
  uint32 len;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  Geometry_buffer buffer;
  Geometry *geom;

  if (is_length_verified()) return get_nbytes();
  /*
    We allow a geometry collection of 0 components, because this is how we
    define an 'empty collection', which is used as the result of set operation
    that returns an empty result, it's different from NULL value just in the
    same way an empty string is different from a NULL.
   */
  if (wkb.scan_non_zero_uint4(&n_objects) && n_objects != 0)
    return GET_SIZE_ERROR;

  while (n_objects--) {
    if (!(geom = scan_header_and_create(&wkb, &buffer))) return GET_SIZE_ERROR;

    uint32 object_size;
    /*
      'geom' is a temporary object whose length may be wrongly specified by
      scan_header_and_create() function, so here we don't require
      object_size + GEOM_HEADER_SIZE == wkb->length()
    */
    if ((object_size = geom->get_data_size()) == GET_SIZE_ERROR)
      return GET_SIZE_ERROR;

    /*
      Use 'skip()' instead of 'skip_unsafe()' in case the object size is
      incorrect
    */
    if (wkb.skip(object_size)) {
      DBUG_ASSERT(false);  // geom-get_data_size() did something wrong.
      return GET_SIZE_ERROR;
    }
  }
  len = static_cast<uint32>(wkb.data() - (const char *)get_data_ptr());
  if (len != get_nbytes()) set_nbytes(len);
  set_length_verified(true);
  return len;
}

bool Gis_geometry_collection::init_from_wkt(Gis_read_stream *trs, String *wkb) {
  if (check_stack_overrun(trs->thd(), STACK_MIN_SIZE, nullptr)) return true;

  uint32 n_objects = 0;
  uint32 no_pos = wkb->length();
  Geometry_buffer buffer;
  Geometry *g;

  if (wkb->reserve(4, 512)) return true;
  wkb->length(wkb->length() + 4);  // Reserve space for points

  if (trs->get_next_toc_type() == Gis_read_stream::word) {
    LEX_CSTRING empty;
    if (trs->get_next_word(&empty) || empty.length != 5 ||
        native_strncasecmp("EMPTY", empty.str, 5))
      return true;
  } else {
    if (trs->check_next_symbol('(')) return true;
    for (;;) {
      /*
        Allow specifying an empty geometry collection in this form:
        'geometrycollection()'.
      */
      if (n_objects == 0 && trs->get_next_toc_type() == Gis_read_stream::r_bra)
        break;
      if (!(g = create_from_wkt(&buffer, trs, wkb, true,
                                false /* Allow trailing bytes. */)))
        return true;
      /*
        Allow g to be a nested geometry collection, nested ones are flatterned
        in BG_geometry_collection before sending to new BG based GIS algorithms.
      */
      n_objects++;
      if (trs->skip_char(','))  // Didn't find ','
        break;
    }
    if (trs->check_next_symbol(')')) return true;
  }

  write_at_position(no_pos, n_objects, wkb);
  return false;
}

uint Gis_geometry_collection::init_from_wkb(THD *thd, const char *wkb, uint len,
                                            wkbByteOrder bo, String *res) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr)) return 0;

  uint32 n_geom = 0;
  const char *wkb_orig = wkb;

  if (len < 4) return 0;
  n_geom = wkb_get_uint(wkb, bo);

  if (res->reserve(4, 512)) return 0;
  q_append(n_geom, res);

  wkb += 4;
  len -= 4;

  /* Allow 0 components as an empty collection. */
  while (n_geom--) {
    Geometry_buffer buffer;
    Geometry *geom;
    uint g_len = 0;
    uint32 wkb_type;

    if (len < WKB_HEADER_SIZE || (*wkb != wkb_xdr && *wkb != wkb_ndr) ||
        res->reserve(WKB_HEADER_SIZE, 512))
      return 0;

    wkb_type = wkb_get_uint(wkb + 1, (wkbByteOrder)wkb[0]);
    write_wkb_header(res, static_cast<wkbType>(wkb_type));

    if (!(geom = create_by_typeid(&buffer, wkb_type)) ||
        !(g_len = geom->init_from_wkb(thd, wkb + WKB_HEADER_SIZE,
                                      len - WKB_HEADER_SIZE,
                                      (wkbByteOrder)wkb[0], res)))
      return 0;
    g_len += WKB_HEADER_SIZE;
    wkb += g_len;
    DBUG_ASSERT(len >= g_len);
    len -= g_len;
  }
  return (uint)(wkb - wkb_orig);
}

bool Gis_geometry_collection::get_data_as_wkt(String *txt,
                                              wkb_parser *wkb) const {
  uint32 n_objects = 0;
  Geometry_buffer buffer;
  Geometry *geom;
  size_t nback = 1;

  /* Allow 0 components as an empty collection. */
  if (wkb->scan_non_zero_uint4(&n_objects) && n_objects != 0) return true;

  if (n_objects == 0) {
    txt->append(STRING_WITH_LEN(" EMPTY"));
  } else {
    txt->append('(');
    while (n_objects--) {
      if (!(geom = scan_header_and_create(wkb, &buffer)) ||
          geom->as_wkt(txt, wkb) || txt->append(STRING_WITH_LEN(","), 512))
        return true;
    }
    txt->length(txt->length() - nback);
    txt->append(')');
  }
  return false;
}

bool Gis_geometry_collection::get_mbr(MBR *mbr, wkb_parser *wkb) const {
  uint32 n_objects;
  Geometry_buffer buffer;
  Geometry *geom;

  /* An empty collection's MBR is NULL. */
  if (wkb->scan_non_zero_uint4(&n_objects)) return true;

  bool found_one = false;
  while (n_objects--) {
    if (!(geom = scan_header_and_create(wkb, &buffer)) ||
        geom->get_mbr(mbr, wkb)) {
      /*
        An empty collection should be simply skipped, it may contain a tree
        of empty collections which is still empty.
      */
      if (geom != nullptr && geom->get_type() == wkb_geometrycollection)
        continue;
      return true;
    }

    // Now we've found a solid component and updated the mbr.
    found_one = true;
  }

  /* An collection containing only a few empty collections, the MBR is NULL. */
  if (!found_one) return true;
  return false;
}

int Gis_geometry_collection::num_geometries(uint32 *num) const {
  *num = 0;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  /* Should return 0 and *num=0 if called with an empty collection. */
  return (wkb.scan_non_zero_uint4(num) && *num != 0) ? 1 : 0;
}

int Gis_geometry_collection::geometry_n(uint32 num, String *result) const {
  uint32 n_objects, length;
  wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
  Geometry_buffer buffer;
  Geometry *geom;

  /* It's an error to call this with an empty collection. */
  if (wkb.scan_non_zero_uint4(&n_objects)) return 1;

  if (num > n_objects || num < 1) return 1;

  wkb_header header;
  do {
    if (wkb.scan_wkb_header(&header) ||
        !(geom = create_by_typeid(&buffer, header.wkb_type)))
      return 1;

    // Must set precise length for points even for a temporary/intermediate one.
    if (geom->get_type() == wkb_point)
      geom->set_data_ptr(wkb.data(), POINT_DATA_SIZE);
    else
      geom->set_data_ptr(&wkb);

    if ((length = geom->get_data_size()) == GET_SIZE_ERROR) return 1;
    wkb.skip_unsafe(length);
  } while (--num);

  /* Copy found object to result */
  if (result->reserve(1 + 4 + length, 512)) return 1;
  q_append((char)wkb_ndr, result);
  q_append(header.wkb_type, result);
  q_append(wkb.data() - length, length, result);  // data-length= start_of_data
  return 0;
}

/*
  Return dimension for object

  SYNOPSIS
    dimension()
    res_dim		Result dimension
    end			End of object will be stored here. May be 0 for
                        simple objects!
  RETURN
    0	ok
    1	error
*/

bool Gis_geometry_collection::dimension(uint32 *res_dim,
                                        wkb_parser *wkb) const {
  uint32 n_objects;
  Geometry_buffer buffer;
  Geometry *geom;

  if (wkb->scan_non_zero_uint4(&n_objects)) return true;

  *res_dim = 0;
  while (n_objects--) {
    uint32 dim;
    if (!(geom = scan_header_and_create(wkb, &buffer)) ||
        geom->dimension(&dim, wkb))
      return true;
    *res_dim = std::max(*res_dim, dim);
  }
  return false;
}

bool Gis_geometry_collection::reverse_coordinates() {
  uint32 num_of_geometries;
  size_t current_data_offset =
      4;  // Add num_of_geometries header size to offset.

  String str(get_cptr(), get_nbytes(), &my_charset_bin);

  if (num_geometries(&num_of_geometries)) {
    return true;
  }

  for (uint32 i = 1; i <= num_of_geometries; i++) {
    String result;

    if (geometry_n(i, &result)) {
      return true;
    }

    Geometry *g;
    Geometry_buffer buffer;
    if (!(g = Geometry::construct(&buffer, &result, false))) {
      return true;
    }

    if (g->reverse_coordinates()) {
      return true;
    }

    if (str.replace(current_data_offset, result.length(), result.ptr(),
                    result.length())) {
      return true;
    }

    current_data_offset += result.length();
  }

  return false;
}

bool Gis_geometry_collection::validate_coordinate_range(
    double srs_angular_unit, bool *long_out_of_range, bool *lat_out_of_range,
    double *out_of_range_value) {
  uint32 num_of_geometries;
  *long_out_of_range = false;
  *lat_out_of_range = false;

  if (num_geometries(&num_of_geometries)) {
    return true; /* purecov: inspected */
  }

  for (uint32 i = 1; i <= num_of_geometries; i++) {
    String result;

    if (geometry_n(i, &result)) {
      return true; /* purecov: inspected */
    }

    Geometry *g;
    Geometry_buffer buffer;
    if (!(g = Geometry::construct(&buffer, &result, false))) {
      return true; /* purecov: inspected */
    }

    if (g->validate_coordinate_range(srs_angular_unit, long_out_of_range,
                                     lat_out_of_range, out_of_range_value)) {
      return true;
    }
  }

  return false;
}

const Geometry::Class_info *Gis_geometry_collection::get_class_info() const {
  return &geometrycollection_class;
}

/************************* Stepper classes  ****************************/

/**
   Base class of all WKB parsers, which parse different types of geometries
   properly. All these classes assume the WKB input is valid and complete.
  */
class Stepper_base {
 public:
  /**
    Constructor.
    @param dim dimension of points in the geometry to be stepped
         over(i.e. current geometry).
    @param bo current geometry's byte order
    @param has_wkb_hdr true for stepping through geometries within multiXXX and
        geometrycollection, false for other geometries.
    @param geotype current geometry's type
  */
  Stepper_base(char dim, Geometry::wkbByteOrder bo, bool has_wkb_hdr,
               Geometry::wkbType geotype) {
    m_dim = dim;
    DBUG_ASSERT(bo == Geometry::wkb_ndr);
    m_bo = bo;
    m_has_wkb_hdr = has_wkb_hdr;
    m_geotype = geotype;
  }

  Geometry::wkbByteOrder get_current_byte_order() const {
    DBUG_ASSERT((m_bo == Geometry::wkb_xdr || m_bo == Geometry::wkb_ndr));
    return m_bo;
  }

  Geometry::wkbType get_current_geotype() const {
    DBUG_ASSERT(Geometry::is_valid_geotype(m_geotype));
    return m_geotype;
  }

 protected:
  /// Current geometry dimension.
  char m_dim;
  /// Current geometry has a WKB header or not.
  bool m_has_wkb_hdr;
  /// Current geometry's byte order.
  Geometry::wkbByteOrder m_bo;
  /// Current geometry's type, e.g. polygon, linestring, etc.
  Geometry::wkbType m_geotype;
};

/**
  For iterating points inside multipoint and linestring.
  Expected multipoint format: NUM-pts|WKB-HDR1 pt1|WKB-HDR2 pt2|...|WKB-HDRn ptn
  Expected linestring format doesn't have the WKB headers.
 */
class Point_stepper : public Stepper_base {
 public:
  Point_stepper(char dim, Geometry::wkbByteOrder bo, bool has_wkb_hdr)
      : Stepper_base(dim, bo, has_wkb_hdr, Geometry::wkb_point) {}

  const char *operator()(const char *p);
};

/**
  For iterating linestrings inside multilinestring and polygon.
  Expected multilinestring format: NUM-ls|WKB-HDR1 ls1|WKB-HDR2 ls2|....
  Expected polygon format doesn't have the WKB headers, and the 1st one is
  exterior ring, following if any are interior rings.

  In both cases, the linestrX is of linestring format, with no WKB header
  in its each point.
*/
class Linestring_stepper : public Stepper_base {
 public:
  Linestring_stepper(char dim, Geometry::wkbByteOrder bo, bool has_wkb_hdr)
      : Stepper_base(dim, bo, has_wkb_hdr, Geometry::wkb_linestring) {}

  const char *operator()(const char *p);
};

/**
  For iterating polygons inside multipolygon or geometry collection.
  Expected multipolygon format: NUM-plgns|WKB-HDR1 plgn1|WKB-HDR2 plgn2|...
  This is also expected format for geometry collection.
  In both cases inside polygonX there is no more WKB headers.
 */
class Polygon_stepper : public Stepper_base {
 public:
  Polygon_stepper(char dim, Geometry::wkbByteOrder bo, bool has_wkb_hdr)
      : Stepper_base(dim, bo, has_wkb_hdr, Geometry::wkb_polygon) {}

  const char *operator()(const char *p);
};

/// Parsing operator. Note that the returned pointer may point past end of
/// WKB string, and caller is responsible for stoping reading after last
/// geometry is read, this is true for all parsing operator of all stepper
/// classes.
/// @param p points to the 1st byte of a point's wkb data, right after its
/// wkb header if any; returns the next point's wkb data's 1st byte pointer,
/// skipping its wkb header if any.
const char *Point_stepper::operator()(const char *p) {
  p += SIZEOF_STORED_DOUBLE * m_dim;

  // m_bo is latest byte order, which allows mixed byte orders in the same
  // wkb byte string.
  if (m_has_wkb_hdr) {
    Geometry::wkbByteOrder bo = get_byte_order(p);

    // The next one can be other geo types, in a geometry collection.
    m_geotype = get_wkb_geotype(p + 1);

    if (m_bo != bo) m_bo = bo;
    p += WKB_HEADER_SIZE;
  }

  return p;
}

/// Parsing operator.
/// @param p points to the 1st byte of a linestring's wkb data, right after
/// its wkb header if any;
/// @return the next linestring's wkb data's 1st
/// byte pointer, skipping its wkb header if any.
const char *Linestring_stepper::operator()(const char *p) {
  uint32 npts = 0;

  npts = uint4korr(p);
  p += sizeof(uint32);
  p += npts * SIZEOF_STORED_DOUBLE * m_dim;

  // The m_bo is latest byte order, which allows mixed byte orders in the same
  // wkb byte string.
  if (m_has_wkb_hdr) {
    Geometry::wkbByteOrder bo = get_byte_order(p);

    // The next one can be other geo types, in a geometry collection.
    m_geotype = get_wkb_geotype(p + 1);

    if (m_bo != bo) m_bo = bo;
    p += WKB_HEADER_SIZE;  // skip the wkb header if any
  }

  return p;
}

/// Parsing operator.
/// @param p points to the 1st byte of a polygon's wkb data, right after its
/// wkb header if any;
/// @return the next polygon's wkb data's 1st byte pointer,
/// skipping its wkb header if any.
const char *Polygon_stepper::operator()(const char *p) {
  uint32 nls = 0;

  // We pass false because a multilinestring's points don't have
  // wkb headers(5 bytes).
  Linestring_stepper lsstepper(m_dim, m_bo, false);

  nls = uint4korr(p);
  p += sizeof(uint32);

  for (uint32 i = 0; i < nls; i++) p = lsstepper(p);

  // m_bo is latest byte order, which allows mixed byte orders in the same
  // wkb byte string.
  DBUG_ASSERT(m_has_wkb_hdr);
  Geometry::wkbByteOrder bo = get_byte_order(p);

  // The next one can be other geo types, in a geometry collection.
  m_geotype = get_wkb_geotype(p + 1);

  if (m_bo != bo) m_bo = bo;
  p += WKB_HEADER_SIZE;  // skip the wkb header if any

  return p;
}

/**
  Get inner rings object from a geometry. Internally check that the argument
  is a polygon. This function is intended as a helper function and is called
  where we don't convert to a polygon pointer although it is a polygon.

  @param g a geometry that must be a polygon.
  @return the polygon's inner rings object.
 */
// SUPPRESS_UBSAN Wrong downcast. FIXME
static inline Gis_polygon::inner_container_type *inner_rings(const Geometry *g)
    SUPPRESS_UBSAN;
static inline Gis_polygon::inner_container_type *inner_rings(
    const Geometry *g) {
  DBUG_ASSERT(g->get_geotype() == Geometry::wkb_polygon);
  const Gis_polygon *p = static_cast<const Gis_polygon *>(g);
  return p->inner_rings();
}

/**
  Set inner rings object to a geometry. Internally check that the argument
  is a polygon. This function is intended as a helper function and is called
  where we don't convert to a polygon pointer although it is a polygon.

  @param g a geometry that must be a polygon.
  @param inns
 */
// SUPPRESS_UBSAN Wrong downcast. FIXME
static inline void set_inner_rings(
    Geometry *g, Gis_polygon::inner_container_type *inns) SUPPRESS_UBSAN;
static inline void set_inner_rings(Geometry *g,
                                   Gis_polygon::inner_container_type *inns) {
  DBUG_ASSERT(g->get_geotype() == Geometry::wkb_polygon);
  Gis_polygon *p = static_cast<Gis_polygon *>(g);
  p->set_inner_rings(inns);
}

/// Parse the wkb buffer to build the component vector m_geo_vect for geom.
/// Set each geometry's wkb pointer into the Geometry objects inside m_geo_vect.
/// Make it a standalone function in order to be able to access classes defined
/// after class template Gis_wkb_vector.
/// @param geom the geometry to analyze and parse.
/// @param p points to the geometry's wkb data's 1st byte, right after its
/// wkb header if any.
/// @param num_geoms number of following geometries, to be used only when
/// parsing the WKB of a polygon's inner rings because there is no WKB header
/// for the inner rings only.
void parse_wkb_data(Geometry *geom, const char *p, size_t num_geoms) {
  const char *q = nullptr;
  size_t nbytes = 0;
  Geometry::wkbType geotype = geom->get_geotype();
  Geometry::wkbByteOrder mybo = geom->get_byte_order();
  char dim = geom->get_dimension();

  DBUG_ASSERT(geotype != Geometry::wkb_polygon_inner_rings ||
              (geotype == Geometry::wkb_polygon_inner_rings && num_geoms != 0));
  geom->set_bg_adapter(true);
  if (p == nullptr) return;

  switch (geotype) {
    case Geometry::wkb_point:
      // Point doesn't need this vector.
      DBUG_ASSERT(false);
      break;
    case Geometry::wkb_linestring: {
      uint32 npts = uint4korr(p);
      p += sizeof(uint32);
      Point_stepper ptstep(dim, mybo, false);
      const char *first = nullptr, *last = nullptr;

      for (uint32 i = 0; i < npts; i++) {
        q = p;
        if (i == 0) first = p;
        if (i < npts - 1) {
          p = ptstep(p);
          nbytes = p - q;
        } else {
          nbytes = geom->get_cptr() + geom->get_nbytes() - p;
          last = p;
        }

        DBUG_ASSERT(nbytes == dim * SIZEOF_STORED_DOUBLE);
        // Construct the geometry object as below to avoid unncesarrily
        // parsing its WKB data. Parsing will be done in shallow_copy.
        Gis_point ent;
        ent.Geometry::set_ptr(q);
        ent.set_nbytes(nbytes);
        ent.set_owner(geom);
        geom->shallow_push(&ent);
      }

      /*
        Historically MySQL GIS require closed polygon rings from user input,
        and from now (5.7.4) on we close a polygon ring if it's open before
        storing/using it. So we should never see any open rings here.

        However we can't compare memory because the two points may have tiny
        differences due to computing deviations.
       */

      // Fix compiler warnings.
      first = last;
      last = first;
      break;
    }
    case Geometry::wkb_multipoint: {
      uint32 npts = uint4korr(p);
      p += sizeof(uint32);

      Geometry::wkbByteOrder bo = ::get_byte_order(p);
      DBUG_ASSERT(get_wkb_geotype(p + 1) == Geometry::wkb_point);
      p += WKB_HEADER_SIZE;

      Point_stepper ptstep(dim, bo, true);

      for (uint32 i = 0; i < npts; i++) {
        q = p;
        if (i < npts - 1) {
          p = ptstep(p);
          nbytes = p - q - WKB_HEADER_SIZE;
        } else
          nbytes = geom->get_cptr() + geom->get_nbytes() - p;

        // Construct the geometry object as below to avoid unncesarrily
        // parsing its WKB data. Parsing will be done in shallow_copy.
        Gis_point ent;
        ent.Geometry::set_ptr(q);
        ent.set_nbytes(nbytes);
        ent.set_owner(geom);
        geom->shallow_push(&ent);
        bo = ptstep.get_current_byte_order();
        DBUG_ASSERT(ptstep.get_current_geotype() == Geometry::wkb_point);
      }

      break;
    }
    case Geometry::wkb_multilinestring: {
      uint32 nls = uint4korr(p);
      p += sizeof(uint32);

      Geometry::wkbByteOrder bo = ::get_byte_order(p);
      DBUG_ASSERT(get_wkb_geotype(p + 1) == Geometry::wkb_linestring);
      p += WKB_HEADER_SIZE;
      Linestring_stepper lsstep(dim, bo, true);

      for (uint32 i = 0; i < nls; i++) {
        q = p;
        if (i < nls - 1) {
          p = lsstep(p);
          nbytes = p - q - WKB_HEADER_SIZE;
        } else
          nbytes = geom->get_cptr() + geom->get_nbytes() - p;

        // Construct the geometry object as below to avoid unncesarrily
        // parsing its WKB data. Parsing will be done in shallow_copy.
        Gis_line_string ent;
        ent.Geometry::set_ptr(q);
        ent.set_nbytes(nbytes);
        ent.set_owner(geom);
        geom->shallow_push(&ent);
        bo = lsstep.get_current_byte_order();
        DBUG_ASSERT(lsstep.get_current_geotype() == Geometry::wkb_linestring);
      }

      break;
    }
    case Geometry::wkb_polygon_inner_rings: {
      /*
        There is no independent WKT for inner rings to parse. Inner rings is
        a component of a polygon thus can't have number of rings in its WKB.
        When inner rings are parsed here, it must already have been filled,
        and we are calling this in methods like push_back, etc,
        thus m_geo_vect has the right number of rings.
       */
      size_t nls = num_geoms;
      Linestring_stepper lsstep(dim, mybo, false);

      for (size_t i = 0; i < nls; i++) {
        q = p;
        if (i < nls - 1) {
          p = lsstep(p);
          nbytes = p - q;
        } else
          nbytes = geom->get_cptr() + geom->get_nbytes() - p;

        // Construct the geometry object as below to avoid unncesarrily
        // parsing its WKB data. Parsing will be done in shallow_copy.
        Gis_polygon_ring ent;
        ent.Geometry::set_ptr(q);
        ent.set_nbytes(nbytes);
        ent.set_owner(geom);
        geom->shallow_push(&ent);
      }

      break;
    }
    case Geometry::wkb_polygon: {
      uint32 nls = uint4korr(p);
      const char *start = p;

      p += sizeof(uint32);

      Linestring_stepper lsstep(dim, mybo, false);

      for (uint32 i = 0; i < nls; i++) {
        q = p;
        if (i < nls - 1) {
          p = lsstep(p);
          nbytes = p - q;
        } else
          nbytes = start + geom->get_nbytes() - p;

        if (i == 0) {
          // Parse outer ring.
          Gis_polygon_ring *outer = outer_ring(geom);
          if (geom->get_ptr() == nullptr) {
            outer = new Gis_polygon_ring(
                q, nbytes, Geometry::Flags_t(Geometry::wkb_linestring, 0),
                geom->get_srid());
            outer->set_props(Geometry::POLYGON_OUTER_RING);
            geom->Geometry::set_ptr(outer);
          } else {
            outer->set_ptr(const_cast<char *>(q), nbytes);
            outer->set_byte_order(mybo);
            outer->set_dimension(dim);
            outer->set_geotype(Geometry::wkb_linestring);
          }

          outer->set_owner(geom);
          /*
            When parsing data to get polygon, the memory is a continuous chunk
            belonging to the owner, or it's an existing WKB buffer. Also true
            for below inner rings.
          */
          outer->set_ownmem(false);
        } else {
          Gis_polygon::inner_container_type *inners = inner_rings(geom);

          if (inners == nullptr) {
            inners = new Gis_polygon::inner_container_type();
            set_inner_rings(geom, inners);
            inners->set_byte_order(mybo);
            inners->set_dimension(dim);
            inners->set_geotype(Geometry::wkb_polygon_inner_rings);
            inners->set_owner(geom);
            inners->set_geo_vect(new Geometry_vector<Gis_polygon_ring>());
            inners->set_ptr(const_cast<char *>(q), 0 /* Accumulated below. */);
            inners->set_ownmem(false);
          }

          // Construct the geometry object as below to avoid unncesarrily
          // parsing its WKB data. Parsing will be done in shallow_push.
          Gis_polygon_ring ent;
          ent.Geometry::set_ptr(q);
          ent.set_nbytes(nbytes);
          ent.set_owner(inners);
          ent.set_props(Geometry::POLYGON_INNER_RING);
          inners->shallow_push(&ent);
          inners->set_nbytes(inners->get_nbytes() + nbytes);
        }
      }
      geom->polygon_is_wkb_form(false);

      break;
    }
    case Geometry::wkb_multipolygon: {
      uint32 nplgns = uint4korr(p);
      p += sizeof(uint32);

      Geometry::wkbByteOrder bo = ::get_byte_order(p);
      DBUG_ASSERT(get_wkb_geotype(p + 1) == Geometry::wkb_polygon);
      p += WKB_HEADER_SIZE;
      Polygon_stepper plgn_step(dim, bo, true);

      for (uint32 i = 0; i < nplgns; i++) {
        q = p;
        if (i < nplgns - 1) {
          p = plgn_step(p);
          nbytes = p - q - WKB_HEADER_SIZE;
        } else
          nbytes = geom->get_cptr() + geom->get_nbytes() - p;

        /*
          Construct the geometry object as below to avoid unncesarrily
          parsing its WKB data. Parsing will be done in shallow_copy.
         */
        Gis_polygon ent;
        ent.Geometry::set_ptr(q);
        ent.set_nbytes(nbytes);
        ent.set_owner(geom);
        geom->shallow_push(&ent);
        // The object 'ent' doesn't have any data of its own.
        ent.donate_data();
        bo = plgn_step.get_current_byte_order();
        DBUG_ASSERT(plgn_step.get_current_geotype() == Geometry::wkb_polygon);
      }

      break;
    }
    case Geometry::wkb_geometrycollection:
      /*
        We never create a Gis_wkb_vector using a geometry collection, because
        BG never uses such a type.
       */
      DBUG_ASSERT(false);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
}

/**
  In place normalize polygons' rings, making outer ring CCW and inner rings CW
  by reversing the ring's points in the WKB buffer inplace. This function can
  not be made a virtual function since BG adapter geometry objects may also
  need it.

  @return the WKB buffer address of the geometry which contains the
          converted WKB data. If geometry data is invalid, returns NULL.
 */
const void *Geometry::normalize_ring_order() {
  Geometry *geo = this;
  bool inval = false;

  if (geo->get_type() == Geometry::wkb_polygon) {
    Gis_polygon bgeo(geo->get_data_ptr(), geo->get_data_size(),
                     geo->get_flags(), geo->get_srid());
    if (bgeo.set_polygon_ring_order()) inval = true;
  } else if (geo->get_type() == Geometry::wkb_multipolygon) {
    Gis_multi_polygon bgeo(geo->get_data_ptr(), geo->get_data_size(),
                           geo->get_flags(), geo->get_srid());

    for (size_t i = 0; i < bgeo.size(); i++)
      if (bgeo[i].set_polygon_ring_order()) {
        inval = true;
        break;
      }
  } else if (geo->get_type() == Geometry::wkb_geometrycollection) {
    /*
      This is impossible because BG doesn't use a geometry collection, and
      we can't create a Gis_wkb_vector<T> with a geometry collection.
    */
    DBUG_ASSERT(false);
  }

  if (inval) return nullptr;
  return geo->get_data_ptr();
}

/**
   Because of resize, a geometry's components may reside not in one chunk,
   some may in the m_ptr's chunk; others have their own memory and only exist
   in m_geo_vect vector, not in ptr's chunk. Also, a constructed polygon's
   data is always not in a chunk and needs to be so when it's pushed into a
   multipolygon/geometry collection.
   Thus in mysql before using the returned geometry, also inside the
   container classes before using the wkb data or clearing m_geo_vect,
   we need to make them inline, i.e. reside in one chunk of memory.
   Can only resize a topmost geometry, thus no recursive reassemling
   to do for now.

   Algorithm:

   Step 1. Structure analysis

   Scan this geometry's components, see whether each of them has its own
   memory, if so it's 'out of line', otherwise it's 'inline'. Note down
   those owning memory in a map M1, for each entry X in the map M1, the
   component's index in the component vector m_geo_vect is used as key;
   The inline chunk of memory right before it which may have any number
   of inline components, and the inline chunk's start and end address pair
   is used as value of the inserted item X. If there is no inline chunk
   before the component, X's pointer range is (0, 0). The inline chunk's
   starting address is well maintained during the scan.


   Step 2. Reassembling

   Allocate enough memory space (the length is accumulated in step 1) as WKB
   buffer and call it GBuf here, then copy the WKB of inline and out-of-line
   geometries into GBuf in original order:
   Go through the map by index order, for each item, copy the WKB chunk
   before it into the WKB buffer, then copy this out-of-line geometry's WKB
   into GBuf.

   Special treatment of polygon: we have to pack its value and store their
   WKB separately into a map GP in step 1, and in step 2 for a polygon,
   get its WKB from GP, and at the end release WKB memory buffers held by
   items of GP.
 */
template <typename T>
void Gis_wkb_vector<T>::reassemble() {
  set_bg_adapter(true);
  Geometry::wkbType geotype = get_geotype();
  if (geotype == Geometry::wkb_point || geotype == Geometry::wkb_polygon ||
      geotype == Geometry::wkb_multipoint || m_geo_vect == nullptr ||
      geotype == Geometry::wkb_linestring || m_geo_vect->size() == 0 ||
      !has_out_of_line_components())
    return;

  if (m_geo_vect == nullptr) m_geo_vect = new Geo_vector;
  typedef std::map<size_t, std::pair<const char *, const char *>> segs_t;
  segs_t segs;
  size_t hdrsz = 0, num = m_geo_vect->size(), prev_in = 0, totlen = 0,
         segsz = 0;
  Geo_vector &vec = *m_geo_vect;
  const char *start = get_cptr(), *end = nullptr, *prev_start = get_cptr();
  std::map<size_t, std::pair<void *, size_t>> plgn_data;
  std::map<size_t, std::pair<void *, size_t>>::iterator plgn_data_itr;
  bool is_inns = (geotype == Geometry::wkb_polygon_inner_rings);

  // True if just passed by a geometry having its own memory and not stored
  // inside owner's memory during the scan.
  bool out = false;
  if (geotype != Geometry::wkb_polygon_inner_rings) hdrsz = WKB_HEADER_SIZE;

  uint32 none = 0;  // Used when all components are out of line.

  // Starting step one of the algorithm --- Structure Analysis.
  for (size_t i = 0; i < num; i++) {
    T *veci = &(vec[i]);
    // Polygons are always(almost) out of line. One with its own memory is
    // always out of line.
    if (veci->get_geotype() == Geometry::wkb_polygon || veci->get_ownmem()) {
      // In case of a polygon, see if it's already inline in a different
      // way from other types of geometries.
      if (veci->get_geotype() == Geometry::wkb_polygon &&
          polygon_is_packed(veci, this)) {
        if (out) {
          out = false;
          DBUG_ASSERT(prev_start == veci->get_ptr());
        }
        prev_in = i;
        continue;
      }

      // Record the bytes before 1st geometry component.
      if (i == 0) {
        if (m_ptr) {
          start = get_cptr();
          end = start + sizeof(uint32) /* num geometrys*/;
        } else if (!is_inns) {
          start = reinterpret_cast<char *>(&none);
          end = start + sizeof(none);
        } else
          start = end = nullptr;
      }
      // The previous geometry is already out of line, or no m_ptr allocated.
      else if (out || !prev_start) {
        start = nullptr;
        end = nullptr;
      } else  // The previous geometry is inline, note down the inline range.
      {
        start = prev_start;
        if (veci->get_geotype() == Geometry::wkb_polygon)
          end = get_packed_ptr(&(vec[prev_in])) + vec[prev_in].get_nbytes();
        else
          end = vec[prev_in].get_cptr() + vec[prev_in].get_nbytes();
        prev_start = end;
        // The 'end' points to the 1st byte of next geometry stored in its
        // owner's memory.
      }

      if (veci->get_geotype() != Geometry::wkb_polygon) {
        // When this geometry is a geometry collection, we need to make its
        // components in one chunk first. Not gonna implement this yet since
        // BG doesn't use geometry collection yet, and consequently no
        // component can be a multipoint/multilinestring/multipolygon or a
        // geometrycollection. And multipoint components are already supported
        // so not forbidding them here.
#if !defined(DBUG_OFF)
        Geometry::wkbType veci_gt = veci->get_geotype();
#endif
        DBUG_ASSERT(veci_gt != wkb_geometrycollection &&
                    veci_gt != wkb_multilinestring &&
                    veci_gt != wkb_multipolygon);
        /* A point/multipoint/linestring is always in one memory chunk. */
        totlen += veci->get_nbytes() + hdrsz;
      } else {
        // Must be a polygon out of line.
        size_t nbytes = 0;
        void *plgn_base = get_packed_ptr(veci, &nbytes);
        DBUG_ASSERT(veci->get_nbytes() == 0 || veci->get_nbytes() == nbytes);
        veci->set_nbytes(nbytes);
        plgn_data.insert(std::make_pair(i, std::make_pair(plgn_base, nbytes)));
        totlen += nbytes + hdrsz;
      }

      segs.insert(std::make_pair(i, std::make_pair(start, end)));
      out = true;
    } else {
      if (out) {
        out = false;
        DBUG_ASSERT(prev_start == veci->get_ptr());
      }
      prev_in = i;
    }
  }

  segsz = segs.size();
  if (segsz == 0) {
    has_out_of_line_components(false);
    return;
  }

  size_t nbytes = get_nbytes();
  DBUG_ASSERT((nbytes == 0 && m_ptr == nullptr && num == segsz) ||
              (nbytes > 0 && num >= segsz));

  // If all are out of line, m_ptr is 0 and no room for ring count, otherwise
  // the space for ring count is already counted above.
  totlen += (nbytes ? nbytes : (is_inns ? 0 : sizeof(uint32)));

  size_t len = 0, total_len = 0, last_i = 0, numgeoms = 0;
  // Allocate extra space as free space for the WKB buffer, and write it as
  // defined pattern.
  const size_t extra_wkb_free_space = 32;
  char *ptr = static_cast<char *>(gis_wkb_alloc(totlen + extra_wkb_free_space));
  // The header(object count) is already copied.
  char *q = ptr;

  if (ptr == nullptr) {
    clear_wkb_data();
    m_ptr = nullptr;
    set_nbytes(0);
    set_ownmem(false);
    goto exit;
  }
  memset(ptr + totlen, 0xff, extra_wkb_free_space - 1);
  ptr[totlen + extra_wkb_free_space - 1] = '\0';

  // Starting step two of the algorithm --- Reassembling.
  // Assemble the ins and outs into a single chunk.
  for (segs_t::iterator itr = segs.begin(); itr != segs.end(); ++itr) {
    size_t i = itr->first;
    start = itr->second.first;
    end = itr->second.second;
    const Geometry *veci = &(vec[i]);
    last_i = i;

    // Copy the inline geometries before veci into buffer.
    if (start) {
      memcpy(q, start, len = end - start);
      q += len;
      total_len += len;
    }

    // Set WKB header. This geometry must be one of multilinestring,
    // multipolygon or a polygon's inner rings.
    if (get_geotype() != Geometry::wkb_polygon_inner_rings) {
      q = write_wkb_header(q, veci->get_geotype());
      total_len += hdrsz;
    }

    // Copy the out of line geometry into buffer. A polygon's data isn't
    // packed inside itself, we've packed it and recorded it in plgn_data.
    plgn_data_itr = plgn_data.find(i);
    if (veci->get_geotype() != Geometry::wkb_polygon) {
      DBUG_ASSERT(plgn_data_itr == plgn_data.end());
      len = veci->get_nbytes();
      memcpy(q, veci->get_ptr(), len);
    } else {
      DBUG_ASSERT(plgn_data_itr != plgn_data.end());
      len = plgn_data_itr->second.second;
      memcpy(q, plgn_data_itr->second.first, len);
    }
    q += len;
    total_len += len;
  }

  // There may be trailing inline geometries to copy at old tail.
  if (last_i < vec.size() - 1) {
    len = get_cptr() + get_nbytes() - prev_start;
    memcpy(q, prev_start, len);
    total_len += len;
  }
  DBUG_ASSERT(total_len == totlen);

  // Inner rings doesn't have ring count.
  if (!is_inns) {
    DBUG_ASSERT(segsz + uint4korr(ptr) <= 0xFFFFFFFF);
    int4store(reinterpret_cast<uchar *>(ptr),
              uint4korr(ptr) + static_cast<uint32>(segsz));
  }

  numgeoms = m_geo_vect->size();
  clear_wkb_data();
  set_ptr(ptr, totlen);
  // An inner ring isn't parsed in set_ptr, has to parse separately since
  // we don't know its number of rings.
  if (is_inns) parse_wkb_data(this, get_cptr(), numgeoms);
  set_ownmem(true);
exit:
  for (plgn_data_itr = plgn_data.begin(); plgn_data_itr != plgn_data.end();
       ++plgn_data_itr)
    gis_wkb_free(plgn_data_itr->second.first);

  has_out_of_line_components(false);
}

/// @brief Constructor.
/// @param ptr points to the geometry's wkb data's 1st byte, right after its
/// wkb header if any.
/// @param nbytes the byte order indicated by ptr's wkb header.
/// @param flags
/// @param srid
/// @param is_bg_adapter Whether this object is created to be used by
///        Boost Geometry, or to be only used in MySQL code.
template <typename T>
Gis_wkb_vector<T>::Gis_wkb_vector(const void *ptr, size_t nbytes,
                                  const Flags_t &flags, gis::srid_t srid,
                                  bool is_bg_adapter)
    : Geometry(ptr, nbytes, flags, srid) {
  DBUG_ASSERT((ptr != nullptr && nbytes > 0) ||
              (ptr == nullptr && nbytes == 0));
  set_ownmem(false);  // We use existing WKB data and don't own that memory.
  set_bg_adapter(is_bg_adapter);
  m_geo_vect = nullptr;

  if (!is_bg_adapter) return;

  std::unique_ptr<Geo_vector> guard;

  wkbType geotype = get_geotype();
  // Points don't need it, polygon creates it when parsing.
  if (geotype != Geometry::wkb_point && geotype != Geometry::wkb_polygon &&
      ptr != nullptr)
    guard.reset(m_geo_vect = new Geo_vector());
  // For polygon parsing to work
  if (geotype == Geometry::wkb_polygon) m_ptr = nullptr;

  // Why: wkb_polygon_inner_rings should parse in polygon as a whole.
  // Don't call get_cptr() here, it returns NULL.
  if (geotype != Geometry::wkb_polygon_inner_rings && ptr != nullptr)
    parse_wkb_data(this, static_cast<const char *>(ptr));

  guard.release();
}

template <typename T>
Gis_wkb_vector<T>::Gis_wkb_vector(const Gis_wkb_vector<T> &v)
    : Geometry(v), m_geo_vect(nullptr) {
  DBUG_ASSERT(
      (v.get_ptr() != nullptr && v.get_nbytes() > 0) ||
      (v.get_ptr() == nullptr && !v.get_ownmem() && v.get_nbytes() == 0));
  if (!v.is_bg_adapter() || (v.get_ptr() == nullptr && v.m_geo_vect == nullptr))
    return;
  m_geo_vect = new Geo_vector();
  std::unique_ptr<Geo_vector> guard(m_geo_vect);

  const_cast<self &>(v).reassemble();
  set_flags(v.get_flags());
  set_nbytes(v.get_nbytes());
  if (get_nbytes() > 0) {
    m_ptr = gis_wkb_alloc(v.get_nbytes() + 2);
    if (m_ptr == nullptr) {
      m_geo_vect = nullptr;
      set_ownmem(false);
      set_nbytes(0);
      return;
    }
    memcpy(m_ptr, v.get_ptr(), v.get_nbytes());
    /*
      The extra 2 bytes makes the buffer usable by get_nbytes_free.
      It's hard to know how many more space will be needed so let's
      allocate more later.
    */
    get_cptr()[get_nbytes()] = '\xff';
    get_cptr()[get_nbytes() + 1] = '\0';
    parse_wkb_data(this, get_cptr(), v.get_geo_vect()->size());
    set_ownmem(true);
  }
  guard.release();
}

/**
  Deep assignment from vector 'rhs' to this object.
  @param rhs the Gis_wkb_vector<T> instance to duplicate from.
*/
template <typename T>
Gis_wkb_vector<T> &Gis_wkb_vector<T>::operator=(const Gis_wkb_vector<T> &rhs) {
  if (this == &rhs) return *this;
  Geometry::operator=(rhs);

  DBUG_ASSERT((m_ptr != nullptr && get_ownmem() && get_nbytes() > 0) ||
              (m_ptr == nullptr && !get_ownmem() && get_nbytes() == 0));
  DBUG_ASSERT(
      (rhs.get_ptr() != nullptr && rhs.get_nbytes() > 0) ||
      (rhs.get_ptr() == nullptr && !rhs.get_ownmem() && rhs.get_nbytes() == 0));

  if (m_owner == nullptr) m_owner = rhs.get_owner();

  size_t nbytes_free = get_nbytes_free();
  clear_wkb_data();

  if (rhs.get_ptr() == nullptr) {
    if (m_ptr != nullptr) gis_wkb_free(m_ptr);
    m_ptr = nullptr;
    set_flags(rhs.get_flags());
    return *this;
  }

  /*
    Geometry v may have out of line components, need to reassemble first.
   */
  const_cast<self &>(rhs).reassemble();

  /*
    If have no enough space, reallocate with extra space padded with required
    bytes;
   */
  if (m_ptr == nullptr || get_nbytes() + nbytes_free < rhs.get_nbytes()) {
    gis_wkb_free(m_ptr);
    m_ptr = gis_wkb_alloc(rhs.get_nbytes() + 32 /* some extra space. */);
    if (m_ptr == nullptr) {
      /*
        This object in this case is valid although it doesn't have any data.
       */
      set_nbytes(0);
      set_ownmem(false);
      return *this;
    }

    // Fill extra space with pattern defined by
    // Gis_wkb_vector<>::get_nbytes_free().
    char *cp = get_cptr();
    memset(cp + rhs.get_nbytes(), 0xFF, 32);
    cp[rhs.get_nbytes() + 31] = '\0';
  }

  /*
    If need less space than before, set remaining bytes to 0xFF as requred
    by Gis_wkb_vector<>::get_nbytes_free.
   */
  if (get_nbytes() > rhs.get_nbytes())
    memset(get_cptr() + rhs.get_nbytes(), 0xFF,
           get_nbytes() - rhs.get_nbytes());

  memcpy(m_ptr, rhs.get_ptr(), rhs.get_nbytes());

  set_flags(rhs.get_flags());
  set_ownmem(true);

  m_geo_vect = new Geo_vector();
  parse_wkb_data(this, get_cptr());
  return *this;
}

/**
  The copy constructors of Geometry classes always do deep copy, but when
  pushing a Geometry object into its owner's geo.m_geo_vect, we want to do
  shallow copy because we want all elements in geo.m_geo_vect vector point
  into locations in the geo.m_ptr buffer. In such situations call this
  function.
  @param g   The Geometry object to push into vec.
 */
template <typename T>
void Gis_wkb_vector<T>::shallow_push(const Geometry *g) {
  const T &geo = *(static_cast<const T *>(g));
  T *pgeo = nullptr;

  if (m_geo_vect == nullptr) m_geo_vect = new Geo_vector();
  // Allocate space and create an object with its default constructor.
  pgeo = static_cast<T *>(m_geo_vect->append_object());
  DBUG_ASSERT(pgeo != nullptr);
  if (pgeo == nullptr) return;

  pgeo->set_flags(geo.get_flags());
  pgeo->set_srid(geo.get_srid());
  pgeo->set_bg_adapter(true);
  // Such a shallow copied object never has its own memory regardless of geo.
  pgeo->set_ownmem(false);

  // This will parse and set up pgeo->m_geo_vect properly.
  // Do not copy elements from geo.m_geo_vect into that of pgeo
  // otherwise STL does deep copy using the Geometry copy constructor.
  pgeo->set_ptr(geo.get_ptr(), geo.get_nbytes());
  pgeo->set_owner(geo.get_owner());
}

template <typename T>
void Gis_wkb_vector<T>::set_ptr(void *ptr, size_t len) {
  DBUG_ASSERT(!(ptr == nullptr && len > 0));
  set_bg_adapter(true);
  if (get_geotype() != Geometry::wkb_polygon) {
    if (get_ownmem() && m_ptr != nullptr) gis_wkb_free(m_ptr);
    m_ptr = ptr;
    if (m_geo_vect) clear_wkb_data();
  }
  set_nbytes(len);
  /* When invoked, this object may or may not have its own memory. */
  if (get_geotype() != Geometry::wkb_polygon_inner_rings && m_ptr != nullptr) {
    if (m_geo_vect == nullptr) m_geo_vect = new Geo_vector();
    parse_wkb_data(this, get_cptr());
  }
}

/**
  Update support
  We suppose updating a geometry can happen in the following ways:
  1. create an empty geo, then append components into it, the geo must
     be a topmost one; a complex geometry such as a multilinestring can be
     seen as a tree of geometry components, and the mlstr is the topmost
     geometry, i.e. the root of the tree, its lstrs are next layer of nodes,
     their points are the 3rd layer of tree nodes. Only the root owns the
     wkb buffer, other components point somewhere into the buffer, and can
     only read the data.

     Polygons are only used by getting its exterior ring or inner rings and
     then work on that/those rings, never used as a whole.

  2. *itr=value, each geo::m_owner can be used to track the topmost
     memory owner, and do reallocation to accormodate the value. This is
     for now not supported, will be if needed.

     So far geometry assignment are only used for point objects in boost
     geometry, thus only Geometry and Gis_point have operator=, no other
     classes need so, and thus there is no need for reallocation.
  3. call resize() to append some objects at the end, then assign/append
     values to the added objects using push_back. Objects added this way
     are out of line(unless the object is a point), and user need to call
     reassemble() to make them inline, i.e. stored in its owner's memory.
*/

/// Clear geometry data of this object.
template <typename T>
void Gis_wkb_vector<T>::clear() {
  if (!m_geo_vect) {
    DBUG_ASSERT(m_ptr == nullptr);
    return;
  }

  DBUG_ASSERT(m_geo_vect && get_geotype() != Geometry::wkb_polygon);

  // Keep the component vector because this object can be reused again.
  const void *ptr = get_ptr();
  set_bg_adapter(true);

  if (ptr && get_ownmem()) {
    gis_wkb_free(const_cast<void *>(ptr));
    set_ownmem(false);
  }

  m_ptr = nullptr;
  clear_wkb_data();
  set_nbytes(0);
}

/// Returns payload number of bytes of the topmost geometry holding this
/// geometry, i.e. the memory owner.
template <typename T>
size_t Gis_wkb_vector<T>::current_size() const {
  // Polygon's data may not stay in a continuous chunk, and we update
  // its data using the outer/inner rings.
  DBUG_ASSERT(get_geotype() != Geometry::wkb_polygon);
  set_bg_adapter(true);
  if (m_geo_vect == nullptr || m_geo_vect->empty()) return 0;

  return get_nbytes();
}

/// Get number of free bytes in the buffer held by m_ptr. this object must be
/// an topmost geometry which owns memory.
template <typename T>
size_t Gis_wkb_vector<T>::get_nbytes_free() const {
  DBUG_ASSERT((this->get_ownmem() && m_ptr) || (!get_ownmem() && !m_ptr));

  size_t cap = current_size();
  if (cap == 0) {
    DBUG_ASSERT(m_ptr == nullptr);
    return 0;
  }

  const char *p = nullptr, *ptr = get_cptr();
  DBUG_ASSERT(ptr != nullptr);

  /*
    There will always be remaining free space because in push_back, when
    number of free bytes equals needed bytes we will do a realloc.
   */
  for (p = ptr + cap; *p != 0; p++)
    ;

  return p - ptr - cap + 1;
}

template <typename T>
void Gis_wkb_vector<T>::push_back(const T &val) {
  Geometry::wkbType geotype = get_geotype();

  DBUG_ASSERT(geotype != Geometry::wkb_polygon &&
              ((m_ptr && get_ownmem()) || (!m_ptr && !get_ownmem())));

  // Only three possible types of geometries for val, thus no need to
  // do val.reassemble().
  DBUG_ASSERT(val.get_geotype() == wkb_point ||
              val.get_geotype() == wkb_polygon ||
              val.get_geotype() == wkb_linestring);

  DBUG_ASSERT(val.get_ptr() != nullptr);

  size_t cap = 0, nalloc = 0;
  size_t vallen, needed;
  void *src_val = val.get_ptr();

  if (m_geo_vect == nullptr) m_geo_vect = new Geo_vector;
  set_bg_adapter(true);
  vallen = val.get_nbytes();
  /*
    Often inside bg, a polygon is created with no data, then append points
    into outer ring and inner rings, such a polygon is a 'constructed'
    polygon, and in this case we need to assemble
    its data into a continuous chunk.
   */
  if (val.get_geotype() == Geometry::wkb_polygon)
    src_val = get_packed_ptr(&val, &vallen);

  // The 4 types can be resized and have out-of-line components,
  // reassemble first in case we lose them when doing m_geo_vect->clear().
  if (geotype == Geometry::wkb_multilinestring ||
      geotype == Geometry::wkb_geometrycollection ||
      geotype == Geometry::wkb_polygon_inner_rings ||
      geotype == Geometry::wkb_multipolygon)
    reassemble();

  // Get cap only after reassemble().
  cap = current_size();

  needed = vallen + WKB_HEADER_SIZE;
  // Use >= instead of > because we always want to have trailing free bytes.
  if (needed >= this->get_nbytes_free()) {
    nalloc = cap + ((needed * 2 > 256) ? needed * 2 : 256);
    void *ptr = get_ptr();
    m_ptr = gis_wkb_realloc(m_ptr, nalloc);
    if (m_ptr == nullptr) {
      set_nbytes(0);
      set_ownmem(0);
      clear_wkb_data();
      return;
    }

    // Set unused space to -1, and last unused byte to 0.
    // Function get_nbytes_free relies on this format.
    memset(get_cptr() + cap, 0xff, nalloc - cap);
    get_cptr()[nalloc - 1] = '\0';
    memset(get_cptr() + cap, 0, sizeof(uint32));

    bool replaced = (ptr != m_ptr);
    set_ownmem(true);
    if (m_owner && m_owner->get_geotype() == Geometry::wkb_polygon)
      m_owner->set_ownmem(true);

    // After reallocation we need to parse again.
    if (cap > 0 && replaced) {
      size_t ngeos = 0;
      if (geotype == Geometry::wkb_polygon_inner_rings) ngeos = size();
      clear_wkb_data();
      parse_wkb_data(this, get_cptr(), ngeos);
    }
  }

  size_t wkb_header_size = 0;
  /* Offset for obj count, if needed. */
  size_t obj_count_len =
      ((cap == 0 && geotype != Geometry::wkb_polygon_inner_rings)
           ? sizeof(uint32)
           : 0);
  char *val_ptr = get_cptr() + cap + obj_count_len;

  // Append WKB header first, if needed.
  if (geotype == Geometry::wkb_multipoint ||
      geotype == Geometry::wkb_multipolygon ||
      geotype == Geometry::wkb_multilinestring ||
      geotype == Geometry::wkb_geometrycollection) {
    Geometry::wkbType vgt = val.get_geotype();
    DBUG_ASSERT(
        (geotype == Geometry::wkb_multipoint && vgt == Geometry::wkb_point) ||
        (geotype == Geometry::wkb_multipolygon &&
         vgt == Geometry::wkb_polygon) ||
        (geotype == Geometry::wkb_multilinestring &&
         vgt == Geometry::wkb_linestring) ||
        geotype == Geometry::wkb_geometrycollection);

    val_ptr = write_wkb_header(val_ptr, vgt);
    wkb_header_size = WKB_HEADER_SIZE;
  }

  // Copy val's data into buffer, then parse it.
  memcpy(val_ptr, src_val, vallen);
  set_nbytes(get_nbytes() + wkb_header_size + obj_count_len + vallen);

  // Append geometry component into m_geo_vect vector. Try to avoid
  // unnecessary parse by calling the right version of set_ptr. And do
  // shallow push so that the element in m_geo_vect point to WKB buffer
  // rather than have its own copy of the same WKB data.
  T val2;
  val2.set_flags(val.get_flags());
  val2.set_srid(val.get_srid());
  val2.Geometry::set_ptr(val_ptr);
  val2.set_nbytes(vallen);
  val2.set_owner(this);
  val2.set_ownmem(false);

  shallow_push(&val2);
  val2.Geometry::set_ptr(nullptr);

  if (val2.get_geotype() == Geometry::wkb_polygon)
    own_rings(&(m_geo_vect->back()));
  if (geotype != Geometry::wkb_polygon_inner_rings) {
    int4store(get_ucptr(), uint4korr(get_ucptr()) + 1);
    DBUG_ASSERT(uint4korr(get_ucptr()) == this->m_geo_vect->size());
  }

  if (val.get_geotype() == Geometry::wkb_polygon) gis_wkb_free(src_val);
}

/*
  Resize as in std::vector<>::resize().

  Because resize can be called to append an empty geometry into its owner,
  we have to allow pushing into an empty geo and its memory will not
  be in the same chunk as its owner, which is OK for bg since the
  Boost Range concept doesn't forbid so. But inside MySQL we should
  reassemble the geometries into one chunk before using the WKB buffer
  directly, by calling reassemble().
*/
template <typename T>
void Gis_wkb_vector<T>::resize(size_t sz) {
  if (m_geo_vect == nullptr) m_geo_vect = new Geo_vector;
  Geometry::wkbType geotype = get_geotype();
  size_t ngeo = m_geo_vect->size();
  size_t dim = GEOM_DIM;
  size_t ptsz = SIZEOF_STORED_DOUBLE * dim;
  bool is_mpt = (geotype == Geometry::wkb_multipoint);

  // Can resize a topmost geometry or a out of line geometry which has
  // or will have its own memory(i.e. one that's not using others' memory).
  // Points are fixed size, polygon doesn't hold data directly.
  DBUG_ASSERT(!(m_ptr != nullptr && !get_ownmem()) &&
              geotype != Geometry::wkb_point &&
              geotype != Geometry::wkb_polygon);
  set_bg_adapter(true);
  if (sz == ngeo) return;
  // Shrinking the vector.
  if (sz < ngeo) {
    // Some elements may be out of line, must do so otherwise we don't
    // know how much to shrink in m_ptr.
    reassemble();
    size_t sublen = 0;
    for (size_t i = ngeo; i > sz; i--)
      sublen += (*m_geo_vect)[i - 1].get_nbytes();

    // '\0' not allowed in middle and no need for ending '\0' because it's
    // at the end of the original free chunk which is right after this chunk.
    memset((get_cptr() + get_nbytes() - sublen), 0xff, sublen);
    set_nbytes(get_nbytes() - sublen);

#if !defined(DBUG_OFF)
    bool rsz_ret = m_geo_vect->resize(sz);
    DBUG_ASSERT(rsz_ret == false);
#else
    m_geo_vect->resize(sz);
#endif
    if (get_geotype() != Geometry::wkb_polygon_inner_rings) {
      DBUG_ASSERT(uint4korr(get_ucptr()) == ngeo);
      int4store(get_ucptr(), static_cast<uint32>(sz));
    }
    return;
  }

  char *ptr = nullptr, *ptr2 = nullptr;

  // We can store points directly into its owner, points are fixed length,
  // thus don't need its own memory.
  if (geotype == Geometry::wkb_linestring ||
      geotype == Geometry::wkb_multipoint) {
    size_t left = get_nbytes_free(),
           needed = (sz - ngeo) * (ptsz + (is_mpt ? WKB_HEADER_SIZE : 0)),
           nalloc, cap = get_nbytes();

    if (left <= needed) {
      nalloc = cap + 32 * (left + needed);
      ptr = get_cptr();
      m_ptr = gis_wkb_realloc(m_ptr, nalloc);
      if (m_ptr == nullptr) {
        set_nbytes(0);
        set_ownmem(0);
        clear_wkb_data();
        return;
      }
      ptr2 = get_cptr();
      memset((ptr2 + cap), 0xff, nalloc - cap);
      ptr2[nalloc - 1] = '\0';
      /*
        Only set when cap is 0, otherwise after this call get_nbytes_free()
        will work wrong, this is different from push_back because push_back
        always put data here more than 4 bytes inside itself.
      */
      if (cap == 0) int4store(get_ucptr(), 0);  // obj count
      set_ownmem(true);

      if (cap > 0 && ptr != m_ptr) {
        clear_wkb_data();
        // Note: flags_.nbytes doesn't change.
        parse_wkb_data(this, get_cptr());
      }
    }
    ptr2 = get_cptr();
    ptr = ptr2 + (cap ? cap : sizeof(uint32) /* obj count */);
    if (cap == 0) set_nbytes(sizeof(uint32));
  } else
    has_out_of_line_components(true);

  /*
    Because the pushed objects have their own memory, here we won't modify
    m_ptr memory at all.
  */
  for (size_t cnt = sz - ngeo; cnt; cnt--) {
    T tmp;
    tmp.set_owner(this);
    tmp.set_ownmem(false);
    // Points are directly put into owner's buffer, no need for own memory.
    if (tmp.get_geotype() == Geometry::wkb_point) {
      if (is_mpt) {
        ptr = write_wkb_header(ptr, Geometry::wkb_point);
        set_nbytes(get_nbytes() + WKB_HEADER_SIZE);
      }
      tmp.set_ptr(ptr, ptsz);
      set_nbytes(get_nbytes() + ptsz);
      ptr += ptsz;
      int4store(get_ucptr(), uint4korr(get_ucptr()) + 1);
      DBUG_ASSERT(uint4korr(get_ucptr()) == m_geo_vect->size() + 1);
    } else
      DBUG_ASSERT(ptr == nullptr && ptr2 == nullptr);

    shallow_push(&tmp);
    if (tmp.get_geotype() == Geometry::wkb_polygon)
      own_rings(&(m_geo_vect->back()));

    // tmp will be filled by push_back after this call, which will make
    // tmp own its own memory, different from other geos in m_geo_vect,
    // this is OK, users should call reassemble() to put them into
    // a single chunk of memory.
  }
}

// Explicit template instantiation
/// @cond
template void Gis_wkb_vector<Gis_line_string>::clear();
template void Gis_wkb_vector<Gis_point>::clear();
template void Gis_wkb_vector<Gis_polygon>::clear();
template void Gis_wkb_vector<Gis_polygon_ring>::clear();

template void Gis_wkb_vector<Gis_line_string>::push_back(
    Gis_line_string const &);
template void Gis_wkb_vector<Gis_point>::push_back(Gis_point const &);
template void Gis_wkb_vector<Gis_polygon>::push_back(Gis_polygon const &);
template void Gis_wkb_vector<Gis_polygon_ring>::push_back(
    Gis_polygon_ring const &);

template void Gis_wkb_vector<Gis_line_string>::reassemble();
template void Gis_wkb_vector<Gis_polygon>::reassemble();

template void Gis_wkb_vector<Gis_line_string>::resize(size_t);
template void Gis_wkb_vector<Gis_point>::resize(size_t);
template void Gis_wkb_vector<Gis_polygon>::resize(size_t);
template void Gis_wkb_vector<Gis_polygon_ring>::resize(size_t);

template Gis_wkb_vector<Gis_line_string>::Gis_wkb_vector(
    const void *, size_t, const Geometry::Flags_t &, gis::srid_t, bool);
template Gis_wkb_vector<Gis_polygon>::Gis_wkb_vector(const void *, size_t,
                                                     const Geometry::Flags_t &,
                                                     gis::srid_t, bool);
template Gis_wkb_vector<Gis_point>::Gis_wkb_vector(const void *, size_t,
                                                   const Geometry::Flags_t &,
                                                   gis::srid_t, bool);

template Gis_wkb_vector<Gis_point> &Gis_wkb_vector<Gis_point>::operator=(
    Gis_wkb_vector<Gis_point> const &);

template Gis_wkb_vector<Gis_point>::Gis_wkb_vector(
    Gis_wkb_vector<Gis_point> const &);
template Gis_wkb_vector<Gis_polygon>::Gis_wkb_vector(
    const Gis_wkb_vector<Gis_polygon> &);
/// @endcond
