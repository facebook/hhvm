/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SPATIAL_INCLUDED
#define SPATIAL_INCLUDED

#include <float.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <cstddef>
#include <iterator>

#include "lex_string.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/gis/srid.h"
#include "sql/inplace_vector.h"
#include "sql_string.h"  // String
#include "unsafe_string_append.h"

class Gis_read_stream;
class THD;

const uint GEOM_DIM = 2;
const uint SRID_SIZE = 4;
const uint SIZEOF_STORED_DOUBLE = 8;
const uint POINT_DATA_SIZE = (SIZEOF_STORED_DOUBLE * 2);
const uint WKB_HEADER_SIZE = (1 + 4);
const uint GEOM_HEADER_SIZE = (SRID_SIZE + WKB_HEADER_SIZE);

const uint32 GET_SIZE_ERROR = 0xFFFFFFFFU;

/**
  Point with coordinates X and Y.
*/
class point_xy {
 public:
  double x;
  double y;
  point_xy() {}
  point_xy(double x_arg, double y_arg) : x(x_arg), y(y_arg) {}
  double distance(const point_xy &p) const;
  /**
    Compare to another point.
    Return true if equal, false if not equal.
  */
  bool eq(point_xy p) const { return (x == p.x) && (y == p.y); }
};

typedef struct wkb_header_st {
  uchar byte_order;
  uint32 wkb_type;
} wkb_header;

/***************************** MBR *******************************/

struct MBR {
  double xmin, ymin, xmax, ymax;

  MBR() {
    xmin = ymin = DBL_MAX;
    xmax = ymax = -DBL_MAX;
  }

  MBR(const double xmin_arg, const double ymin_arg, const double xmax_arg,
      const double ymax_arg)
      : xmin(xmin_arg), ymin(ymin_arg), xmax(xmax_arg), ymax(ymax_arg) {}

  MBR(const point_xy &min, const point_xy &max)
      : xmin(min.x), ymin(min.y), xmax(max.x), ymax(max.y) {}

  void add_xy(double x, double y) {
    /* Not using "else" for proper one point MBR calculation */
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
  }
  void add_xy(point_xy p) { add_xy(p.x, p.y); }
  void add_xy(const char *px, const char *py) {
    double x = float8get(px);
    double y = float8get(py);
    add_xy(x, y);
  }
  void add_mbr(const MBR *mbr) {
    if (mbr->xmin < xmin) xmin = mbr->xmin;
    if (mbr->xmax > xmax) xmax = mbr->xmax;
    if (mbr->ymin < ymin) ymin = mbr->ymin;
    if (mbr->ymax > ymax) ymax = mbr->ymax;
  }

  int equals(const MBR *mbr) const {
    /* The following should be safe, even if we compare doubles */
    return ((mbr->xmin == xmin) && (mbr->ymin == ymin) && (mbr->xmax == xmax) &&
            (mbr->ymax == ymax));
  }

  int disjoint(const MBR *mbr) const {
    /* The following should be safe, even if we compare doubles */
    return ((mbr->xmin > xmax) || (mbr->ymin > ymax) || (mbr->xmax < xmin) ||
            (mbr->ymax < ymin));
  }

  int intersects(const MBR *mbr) const { return !disjoint(mbr); }

  int touches(const MBR *mbr) const;

  int within(const MBR *mbr) const;

  int contains(const MBR *mbr) const { return mbr->within(this); }

  int covered_by(const MBR *mbr) const {
    /* The following should be safe, even if we compare doubles */
    return ((mbr->xmin <= xmin) && (mbr->ymin <= ymin) && (mbr->xmax >= xmax) &&
            (mbr->ymax >= ymax));
  }

  int covers(const MBR *mbr) const { return mbr->covered_by(this); }

  bool inner_point(double x, double y) const {
    /* The following should be safe, even if we compare doubles */
    return (xmin < x) && (xmax > x) && (ymin < y) && (ymax > y);
  }

  /**
    The dimension maps to an integer as:
    - Polygon -> 2
    - Horizontal or vertical line -> 1
    - Point -> 0
    - Invalid MBR -> -1
  */
  int dimension() const {
    int d = 0;

    if (xmin > xmax)
      return -1;
    else if (xmin < xmax)
      d++;

    if (ymin > ymax)
      return -1;
    else if (ymin < ymax)
      d++;

    return d;
  }

  int overlaps(const MBR *mbr) const {
    /*
      overlaps() requires that some point inside *this is also inside
      *mbr, and that both geometries and their intersection are of the
      same dimension.
    */
    int d = dimension();
    DBUG_ASSERT(d >= 0 && d <= 2);

    if (d != mbr->dimension() || d == 0 || contains(mbr) || within(mbr))
      return 0;

    MBR intersection(std::max(xmin, mbr->xmin), std::max(ymin, mbr->ymin),
                     std::min(xmax, mbr->xmax), std::min(ymax, mbr->ymax));

    return (d == intersection.dimension());
  }
};

/***************************** Geometry *******************************/

struct Geometry_buffer;

/*
  Memory management functions for BG adapter code. Allocate extra space for
  GEOMETRY header so that we can later prefix the header if needed.
 */
void *gis_wkb_alloc(size_t sz);

inline void *gis_wkb_fixed_alloc(size_t sz) { return gis_wkb_alloc(sz); }

void *gis_wkb_realloc(void *p, size_t sz);

inline void gis_wkb_free(void *p) {
  if (p == nullptr) return;
  char *cp = static_cast<char *>(p);
  my_free(cp - GEOM_HEADER_SIZE);
}

inline void gis_wkb_raw_free(void *p) { my_free(p); }

class Geometry {
  friend void parse_wkb_data(Geometry *geom, const char *p, size_t num_geoms);

 protected:
  // Flag bits for m_flags.props.

  /*
    Whether the linestring is a polygon's outer ring, or inner ring.
   */
  const static int POLYGON_OUTER_RING = 0x1;
  const static int POLYGON_INNER_RING = 0x2;

  /*
    Whether the Geometry object is created to be used by Boost Geometry or
    only by MySQL. There are some operations that only work for one type and
    can or must be skipped otherwise. This state is transient and mutable, we
    set it even to a const geometry object.
   */
  const static int IS_BOOST_GEOMETRY_ADAPTER = 0x4;

  /*
    Whether the geometry length is verified, so that we can return the stored
    length without having to parse the WKB again.
   */
  const static int GEOM_LENGTH_VERIFIED = 0x8;

  /*
    Whether the geometry has components stored out of line, see
    Gis_wkb_vector<>::resize for details.
   */
  const static int HAS_OUT_OF_LINE_COMPONENTS = 0x10;

  /*
    Whether the polygon's data is in WKB form, as is so in MySQL, or it's in
    BG form, where the m_ptr points to an outer ring object, and m_inn_rings
    points to the inner rings. See Gis_polygon for more information.
   */
  const static int POLYGON_IN_WKB_FORM = 0x20;

  /*
    whether the geometry's data buffer has space for a GEOMETRY header.
    BG adapter code use gis_wkb_alloc to allocate WKB buffer for Geometry
    objects, they always has such space. Gis_geometry_collection created
    from a single geometry and then appended with more geometries also have
    such space. Those with such space we can simply prefix the GEOMETRY header
    into its buffer without copying its WKB data.
   */
  const static int HAS_GEOM_HEADER_SPACE = 0x40;

  /*
    Whether the multi geometry has overlapped components, if false(the bit set)
    this geometry will be skipped from merge-component operation.
    Effective only for multipolygons, multilinestrings and geometry collections.
    Such geometries returned by BG always has this bit set, i.e. their
    components don't overlap.
  */
  const static int MULTIPOLYGON_NO_OVERLAPPED_COMPS = 0x80;

 public:
  // Check user's transmitted data against these limits.
  const static uint32 MAX_GEOM_WKB_LENGTH = 0x3fffffff;

  const static gis::srid_t default_srid = 0;

  virtual ~Geometry();

  /*
    We have to define a wkb_first and wkb_invalid_type and set them to 0
    because Geometry objects stored in m_geo_vect vector can be constructed
    using the default constructur Geometry() which sets geotype to 0, and
    there are asserts in BG adapter code which asserts geotype is in valid
    range [wkb_first, wkb_last]. Neither items will be treated as valid
    geometry types.

    wkb_first and wkb_last are only intended to be used to express a valid
    range of wkbType values, other items are to be used as real type values.
   */
  enum wkbType {
    wkb_invalid_type = 0,
    wkb_first = 1,
    wkb_point = 1,
    wkb_linestring = 2,
    wkb_polygon = 3,
    wkb_multipoint = 4,
    wkb_multilinestring = 5,
    wkb_multipolygon = 6,
    wkb_geometrycollection = 7,
    /*
      OGC defines 10 more basic geometry types for values 8 to 17, we don't
      support them now so don't define them. And there may be more of
      them defined in the future. Since we will need 5 bits anyway, we grow
      from 31 down to 18 for our extra private types instead of from 18 to 31,
      to avoid potential data file format binary compatibility issues, which
      would occur if OGC defined more basic types and we would support them.
     */
    wkb_polygon_inner_rings = 31,
    wkb_last = 31
  };
  enum wkbByteOrder {
    wkb_xdr = 0, /* Big Endian */
    wkb_ndr = 1, /* Little Endian */
    wkb_invalid
  };
  enum enum_coordinate_reference_system {
    coord_first = 1,
    cartesian = 1,
    coord_last = 1
  };

  static String bad_geometry_data;

  /**
    Constant storage for WKB.
    Encapsulation and the available methods make it impossible
    to update the members of wkb_container once it is initialized.
    The only allowed modification method is set(),
    which fully replaces the previous buffer.
  */
  class wkb_container {
   protected:
    const char *m_data;
    const char *m_data_end;

   public:
    wkb_container() {}
    wkb_container(const char *data, const char *data_end) {
      set(data, data_end);
    }
    void set(const char *data, const char *data_end) {
      m_data = data;
      m_data_end = data_end;
    }
    const char *data() const { return m_data; }
    const char *data_end() const { return m_data_end; }
    uint32 length() const { return (uint32)(m_data_end - m_data); }
    /**
      Check if there's enough data remaining as requested.

      @arg data_amount  data requested

      @return           true if not enough data
    */
    bool no_data(size_t data_amount) const {
      return (m_data + data_amount > m_data_end);
    }

    /**
      Check if there're enough points remaining as requested.

      Need to perform the calculation in logical units, since multiplication
      can overflow the size data type.

      @arg expected_points   number of points expected
      @arg extra_point_space extra space for each point element in the array

      @return               true if there are not enough points
    */
    bool not_enough_points(uint32 expected_points,
                           uint32 extra_point_space = 0) const {
      return (m_data_end < m_data ||
              expected_points > ((m_data_end - m_data) /
                                 (POINT_DATA_SIZE + extra_point_space)));
    }
  };

  /**
    WKB parser, designed to traverse through WKB data from
    beginning of the buffer towards the end using a set
    of scan_xxx(), get_xxx() and skip_xxx() routines,
    with safety tests to avoid going beyond the buffer end.
  */
  class wkb_parser : public wkb_container {
    /* Low level routines to get data of various types */
    void get_uint4(uint32 *number) {
      *number = uint4korr(m_data);  // GIS-TODO: byte order
    }
    void get_float8(double *x) {
      *x = float8get(m_data);  // GIS-TODO: byte order
    }

   public:
    wkb_parser(const char *data, const char *data_end)
        : wkb_container(data, data_end) {}

    /* Routines to skip non-interesting data */
    void skip_unsafe(size_t nbytes) {
      DBUG_ASSERT(!no_data(nbytes));
      m_data += nbytes;
    }
    bool skip(size_t nbytes) {
      if (no_data(nbytes)) return true;
      m_data += nbytes;
      return false;
    }
    bool skip_wkb_header() { return skip(WKB_HEADER_SIZE); }
    bool skip_coord() { return skip(SIZEOF_STORED_DOUBLE); }

    /* Routines to scan wkb header information */
    bool scan_wkb_header(wkb_header *header) {
      if (no_data(WKB_HEADER_SIZE)) return true;
      header->byte_order = (uchar)(*m_data);
      m_data++;
      get_uint4(&header->wkb_type);
      m_data += 4;
      return false;
    }

    /* Routines to scan uint4 information */
    bool scan_uint4(uint32 *number) {
      if (no_data(4)) return true;
      get_uint4(number);
      m_data += 4;
      return false;
    }
    bool scan_non_zero_uint4(uint32 *number) {
      return (scan_uint4(number) || 0 == *number);
    }
    bool scan_n_points_and_check_data(uint32 *n_points,
                                      uint32 extra_point_space = 0) {
      return scan_non_zero_uint4(n_points) ||
             not_enough_points(*n_points, extra_point_space);
    }

    /* Routines to scan coordinate information */
    void scan_xy_unsafe(point_xy *p) {
      DBUG_ASSERT(!no_data(POINT_DATA_SIZE));
      get_float8(&p->x);
      m_data += SIZEOF_STORED_DOUBLE;
      get_float8(&p->y);
      m_data += SIZEOF_STORED_DOUBLE;
    }
    bool scan_xy(point_xy *p) {
      if (no_data(SIZEOF_STORED_DOUBLE * 2)) return true;
      scan_xy_unsafe(p);
      return false;
    }
    bool scan_coord(double *x) {
      if (no_data(SIZEOF_STORED_DOUBLE)) return true;
      get_float8(x);
      m_data += SIZEOF_STORED_DOUBLE;
      return false;
    }
  };

  /** Callback which creates Geometry objects on top of a given placement. */
  typedef Geometry *(*create_geom_t)(char *);

  class Class_info {
   public:
    const LEX_CSTRING m_name;
    int m_type_id;
    create_geom_t m_create_func;
    Class_info(const char *name, int type_id, create_geom_t create_func);
  };

  virtual const Class_info *get_class_info() const { return nullptr; }

  virtual uint32 get_data_size() const { return -1; }

  /* read from trs the wkt string and write into wkb as wkb encoded data. */
  virtual bool init_from_wkt(Gis_read_stream *trs MY_ATTRIBUTE((unused)),
                             String *wkb MY_ATTRIBUTE((unused))) {
    return true;
  }

  /* read from wkb the wkb data and write into res as wkb encoded data. */
  /* returns the length of the wkb that was read */
  virtual uint init_from_wkb(THD *thd MY_ATTRIBUTE((unused)),
                             const char *wkb MY_ATTRIBUTE((unused)),
                             uint len MY_ATTRIBUTE((unused)),
                             wkbByteOrder bo MY_ATTRIBUTE((unused)),
                             String *res MY_ATTRIBUTE((unused))) {
    return 0;
  }

  virtual bool get_data_as_wkt(String *txt MY_ATTRIBUTE((unused)),
                               wkb_parser *wkb MY_ATTRIBUTE((unused))) const {
    return true;
  }
  virtual bool get_mbr(MBR *mbr MY_ATTRIBUTE((unused)),
                       wkb_parser *wkb MY_ATTRIBUTE((unused))) const {
    return true;
  }
  bool get_mbr(MBR *mbr) {
    wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
    return get_mbr(mbr, &wkb);
  }
  virtual bool dimension(uint32 *dim, wkb_parser *wkb) const {
    *dim = feature_dimension();
    uint32 length;
    if ((length = get_data_size()) == GET_SIZE_ERROR) return true;
    wkb->skip(length);
    return false;
  }
  bool dimension(uint32 *dim) const {
    wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
    return dimension(dim, &wkb);
  }
  wkbType get_type() const {
    return static_cast<Geometry::wkbType>(get_class_info()->m_type_id);
  }
  enum_coordinate_reference_system get_coordsys() const { return cartesian; }
  virtual uint32 feature_dimension() const {
    DBUG_ASSERT(false);
    return 0;
  }

  virtual int get_x(double *x MY_ATTRIBUTE((unused))) const { return -1; }
  virtual int get_y(double *y MY_ATTRIBUTE((unused))) const { return -1; }
  virtual int geom_length(double *len MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int is_closed(int *closed MY_ATTRIBUTE((unused))) const { return -1; }
  virtual int num_interior_ring(
      uint32 *n_int_rings MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int num_points(uint32 *n_points MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int num_geometries(uint32 *num MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int copy_points(String *result MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  /* The following 7 functions return geometries in wkb format. */
  virtual int start_point(String *point MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int end_point(String *point MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int exterior_ring(String *ring MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int point_n(uint32 num MY_ATTRIBUTE((unused)),
                      String *result MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int interior_ring_n(uint32 num MY_ATTRIBUTE((unused)),
                              String *result MY_ATTRIBUTE((unused))) const {
    return -1;
  }
  virtual int geometry_n(uint32 num MY_ATTRIBUTE((unused)),
                         String *result MY_ATTRIBUTE((unused))) const {
    return -1;
  }

  /**
    Reverses the coordinates of a geometry.

    Switches the coordinates of the wkb string pointed to by the Geometry.
    Ex: Used on a POINT(5,2), the result would be POINT(2, 5).

    @retval false coordinate reversal was successful
    @retval true coordinate reversal was unsuccessful
  */
  virtual bool reverse_coordinates() = 0;

  /**
    Check that the coordinates of a geometry is within the valid range.

    Checks if the coordinates in a geometry are within allowed range of a
    geographic spatial reference system. Valid range for longitude and latitude
    coordinates in geographic spatial reference systems are (-180, 180) and
    [-90, 90] degrees, respectively.

    @param[in] srs_angular_unit Unit to radians conversion factor.
    @param[out] long_out_of_range Longitude is out of range.
    @param[out] lat_out_of_range Latitude is out of range.
    @param[out] out_of_range_value The value that is out of range.

    @retval false Coordinates are within allowed range.
    @retval true Coordinates are not within allowed range, or an error occurred
    during range checking.
  */
  virtual bool validate_coordinate_range(double srs_angular_unit,
                                         bool *long_out_of_range,
                                         bool *lat_out_of_range,
                                         double *out_of_range_value) = 0;

 public:
  static Geometry *create_by_typeid(Geometry_buffer *buffer, int type_id);

  static Geometry *construct(Geometry_buffer *buffer, const char *data,
                             uint32 data_len, bool has_srid = true);
  static Geometry *construct(Geometry_buffer *buffer, const String *str,
                             bool has_srid = true) {
    return construct(buffer, str->ptr(), static_cast<uint32>(str->length()),
                     has_srid);
  }
  static Geometry *create_from_wkt(Geometry_buffer *buffer,
                                   Gis_read_stream *trs, String *wkt,
                                   bool init_stream = true,
                                   bool check_trailing = true);
  static Geometry *create_from_wkb(THD *thd, Geometry_buffer *buffer,
                                   const char *wkb, uint32 len, String *res,
                                   bool init);
  bool as_wkt(String *wkt, wkb_parser *wkb) const {
    uint32 len = (uint)get_class_info()->m_name.length;
    if (wkt->reserve(len + 2, 512)) return true;
    if (get_type() == wkb_geometrycollection)
      wkt->append("GEOMETRYCOLLECTION");
    else
      qs_append(get_class_info()->m_name.str, len, wkt);
    if (get_data_as_wkt(wkt, wkb)) return true;
    return false;
  }
  bool as_wkt(String *wkt) const {
    wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
    return as_wkt(wkt, &wkb);
  }

  bool as_wkb(String *wkb, bool shallow_copy) const;
  bool as_geometry(String *wkb, bool shallow_copy) const;

  void set_data_ptr(const void *data, size_t data_len) {
    m_ptr = const_cast<void *>(data);
    set_nbytes(data_len);
  }

  void set_data_ptr(const wkb_container *c) {
    m_ptr = const_cast<void *>(static_cast<const void *>(c->data()));
    set_nbytes(c->length());
  }
  void *get_data_ptr() const { return m_ptr; }

  bool envelope(String *result) const;
  bool envelope(MBR *mbr) const;

  static Class_info *ci_collection[wkb_last + 1];

  bool is_polygon_ring() const {
    return m_flags.props & (POLYGON_OUTER_RING | POLYGON_INNER_RING);
  }

  bool is_polygon_outer_ring() const {
    return m_flags.props & POLYGON_OUTER_RING;
  }

  bool is_polygon_inner_ring() const {
    return m_flags.props & POLYGON_INNER_RING;
  }

  bool has_geom_header_space() const {
    return (m_flags.props & HAS_GEOM_HEADER_SPACE) ||
           (m_flags.props & IS_BOOST_GEOMETRY_ADAPTER);
  }

  void has_geom_header_space(bool b) {
    if (b)
      m_flags.props |= HAS_GEOM_HEADER_SPACE;
    else
      m_flags.props &= ~HAS_GEOM_HEADER_SPACE;
  }

  bool is_components_no_overlapped() const {
    return (m_flags.props & MULTIPOLYGON_NO_OVERLAPPED_COMPS);
  }

  void set_components_no_overlapped(bool b) {
    DBUG_ASSERT(get_type() == wkb_multilinestring ||
                get_type() == wkb_multipolygon ||
                get_type() == wkb_geometrycollection);
    if (b)
      m_flags.props |= MULTIPOLYGON_NO_OVERLAPPED_COMPS;
    else
      m_flags.props &= ~MULTIPOLYGON_NO_OVERLAPPED_COMPS;
  }

  void set_props(uint16 flag) {
    DBUG_ASSERT(0xfff >= flag);
    m_flags.props |= flag;
  }

  uint16 get_props() const { return (uint16)m_flags.props; }

  void set_srid(gis::srid_t id) { m_srid = id; }

  gis::srid_t get_srid() const { return m_srid; }

  const void *normalize_ring_order();

 protected:
  static Class_info *find_class(int type_id) {
    return ((type_id < wkb_first) || (type_id > wkb_last))
               ? nullptr
               : ci_collection[type_id];
  }
  static Class_info *find_class(const char *name, size_t len);
  void append_points(String *txt, uint32 n_points, wkb_parser *wkb,
                     uint32 offset, bool bracket_pt = false) const;
  bool create_point(String *result, wkb_parser *wkb) const;
  bool create_point(String *result, point_xy p) const;
  bool get_mbr_for_points(MBR *mbr, wkb_parser *wkb, uint offset) const;
  bool is_length_verified() const {
    return m_flags.props & GEOM_LENGTH_VERIFIED;
  }

  // Have to make this const because it's called in a const member function.
  void set_length_verified(bool b) const {
    if (b)
      m_flags.props |= GEOM_LENGTH_VERIFIED;
    else
      m_flags.props &= ~GEOM_LENGTH_VERIFIED;
  }

  /***************************** Boost Geometry Adapter Interface ************/
 public:
  /**
    Highest byte is stores byte order, dimension, nomem and geotype as follows:
    bo: byte order, 1 for little endian(ndr), 0 for big endian(xdr); Currently
        it must be always wkb_ndr since it is MySQL's portable geometry format.
    dimension: 0~3 for 1~4 dimensions;
    nomem: indicating whether this object has its own memory.
           If so, the memory is released when the object is destroyed. Some
           objects may refer to an existing WKB buffer and access it read only.
    geotype: stores the wkbType enum numbers, at most 32 values, valid range
             so far: [0, 7] and 31.

    nybytes: takes the following 30 bits, stores number of effective and valid
             data bytes of current object's wkb data.

    props: bits OR'ed for various other runtime properties of the geometry
           object. Bits are defined above. No properties are stored
           permanently, all properties here are specified/used at runtime
           while the Geometry object is alive.
    zm: not used now, always be 0, i.e. always 2D geometries. In future,
        they represent Z and/or M settings, 1: Z, 2: M, 3: ZM.
    unused: reserved for future use, it's unused now.
  */
  class Flags_t {
   public:
    Flags_t() {
      memset(this, 0, sizeof(*this));
      bo = wkb_ndr;
      dim = GEOM_DIM - 1;
      nomem = 1;
    }

    Flags_t(wkbType type, size_t len) {
      memset(this, 0, sizeof(*this));
      geotype = type;
      nbytes = len;
      bo = wkb_ndr;
      dim = GEOM_DIM - 1;
      nomem = 1;
    }

    uint64 bo : 1;
    uint64 dim : 2;
    uint64 nomem : 1;
    uint64 geotype : 5;
    uint64 nbytes : 30;
    uint64 props : 12;
    uint64 zm : 2;
    uint64 unused : 11;
  };
  static_assert(sizeof(Flags_t) == sizeof(uint64),
                "Flags are expected to line up exactly with an uint64.");

  Geometry() {
    m_ptr = nullptr;
    m_owner = nullptr;
    set_ownmem(false);
    set_byte_order(Geometry::wkb_ndr);
    set_srid(default_srid);
  }

  /**
    Constructor used as BG adapter or by default constructors of children
    classes.
    @param ptr WKB buffer address, or NULL for an empty object.
    @param len WKB buffer length in bytes.
    @param flags the flags to set, no field is used for now except geotype.
    @param srid srid of the geometry.
  */
  Geometry(const void *ptr, size_t len, const Flags_t &flags,
           gis::srid_t srid) {
    m_ptr = const_cast<void *>(ptr);
    m_flags.nbytes = len;
    set_srid(srid);
    m_flags.geotype = flags.geotype;
    m_owner = nullptr;
    set_ownmem(false);
  }

  Geometry(const Geometry &geo);

  Geometry &operator=(const Geometry &rhs);

  /* Getters and setters. */
  void *get_ptr() const { return m_ptr; }

  char *get_cptr() const { return static_cast<char *>(m_ptr); }

  uchar *get_ucptr() const { return static_cast<uchar *>(m_ptr); }

  Geometry *get_owner() const { return m_owner; }

  void set_owner(Geometry *o) { m_owner = o; }

  void set_byte_order(Geometry::wkbByteOrder bo) {
    DBUG_ASSERT(bo == Geometry::wkb_ndr);
    m_flags.bo = static_cast<char>(bo);
  }

  void set_dimension(char dim) {
    // Valid dim is one of [1, 2, 3, 4].
    DBUG_ASSERT(dim > 0 && dim < 5);
    m_flags.dim = dim - 1;
  }

  /**
    Check if a given geometry type is a valid geometry type according
    to OpenGIS.

    Internal geometry types of MySQL are regarded as invalid.

    @param gtype geometry type to check

    @retval true valid geometry type
    @retval false invalid geometry type
  */
  static bool is_valid_opengis_geotype(uint32 gtype) {
    return gtype >= wkb_first && gtype <= wkb_geometrycollection;
  }

  /**
    Check if a given geometry type is a valid internal geometry type.

    Both OpenGIS standard geometry types and internal geometry types
    of MySQL are regarded as valid.

    @param gtype geometry type to check

    @retval true valid geometry type
    @retval false invalid geometry type
  */
  static bool is_valid_geotype(uint32 gtype) {
    /*
      Stricter check, outside only checks for [wkb_first, wkb_last],
      they don't have to know about the details.
     */
    return ((gtype >= wkb_first && gtype <= wkb_geometrycollection) ||
            gtype == wkb_polygon_inner_rings);
  }

  /**
    Check if a given geometry type is a valid internal geometry type.

    Both OpenGIS standard geometry types and internal geometry types
    of MySQL are regarded as valid.

    @param gt geometry type to check

    @retval true valid geometry type
    @retval false invalid geometry type
  */
  static bool is_valid_geotype(Geometry::wkbType gt) {
    /*
      Stricter check, outside only checks for [wkb_first, wkb_last],
      they don't have to know about the details.
     */
    return ((gt >= wkb_first && gt <= wkb_geometrycollection) ||
            gt == wkb_polygon_inner_rings);
  }

  /**
    Verify that a string is a well-formed GEOMETRY string.

    This does not check if the geometry is geometrically valid.

    @see Geometry_well_formed_checker

    @param from String to check
    @param length Length of string
    @param type Expected type of geometry, or
           Geoemtry::wkb_invalid_type if any type is allowed

    @param bo
    @return True if the string is a well-formed GEOMETRY string,
            false otherwise
   */
  static bool is_well_formed(const char *from, size_t length, wkbType type,
                             wkbByteOrder bo);

  void set_geotype(Geometry::wkbType gt) {
    is_valid_geotype(gt);
    m_flags.geotype = static_cast<char>(gt);
  }

  // Have to make this const because it's called in a const member function.
  void set_nbytes(size_t n) const {
    if (get_nbytes() != n) {
      set_length_verified(false);
      m_flags.nbytes = n;
    }
  }

  /**
    Set whether this object has its own memory. If so, the memory is released
    when this object is destroyed.
    @param b true if this object has its own memory, false otherwise.

   */
  void set_ownmem(bool b) { m_flags.nomem = (b ? 0 : 1); }

  /**
    Returns whether this object has its own memory. If so, the memory is
    released when this object is destroyed.
    */
  bool get_ownmem() const { return !m_flags.nomem; }

  Geometry::wkbByteOrder get_byte_order() const {
    DBUG_ASSERT(m_flags.bo == 1);
    return Geometry::wkb_ndr;
  }

  char get_dimension() const { return static_cast<char>(m_flags.dim) + 1; }

  Geometry::wkbType get_geotype() const {
    char gt = static_cast<char>(m_flags.geotype);
    return static_cast<Geometry::wkbType>(gt);
  }

  /**
    Build an OGC standard type value from m_flags.zm and m_flags.geotype. For
    now m_flags.zm is always 0 so simply call get_geotype(). We don't
    directly store the OGC defined values in order to save more bits
    of m_flags for other purposes; and also separating zm settings from basic
    geometry types is easier for coding and geometry type identification.

    When we start to support Z/M settings we need to modify all code which call
    write_wkb_header and write_geometry_header to pass to them an OGC standard
    type value returned by this function or built similarly. And by doing so
    our internal runtime geometry type encoding will work consistently with
    OGC defined standard geometry type values in byte strings of WKB format.

    @return OGC standard geometry type value.
   */
  uint32 get_ogc_geotype() const { return static_cast<uint32>(get_geotype()); }

  size_t get_nbytes() const { return static_cast<size_t>(m_flags.nbytes); }

  /*
    Only sets m_ptr, different from the overloaded one in Gis_wkb_vector<>
    which also does WKB parsing.
   */
  void set_ptr(const void *ptr) { m_ptr = const_cast<void *>(ptr); }

  /**
    Whether the Geometry object is created to be used by Boost Geometry or
    only by MySQL. There are some operations that only work for one type and
    can or must be skipped otherwise.
    @return true if it's a BG adapter, false otherwise.
   */
  bool is_bg_adapter() const {
    return m_flags.props & IS_BOOST_GEOMETRY_ADAPTER;
  }

  /**
    Set whether this object is a BG adapter.
    @param b true if it's a BG adapter, false otherwise.
    Have to declare this as const because even when a Geometry object's const
    adapter member function is called, it's regarded as a BG adapter object.
   */
  void set_bg_adapter(bool b) const {
    if (b)
      m_flags.props |= IS_BOOST_GEOMETRY_ADAPTER;
    else
      m_flags.props &= ~IS_BOOST_GEOMETRY_ADAPTER;
  }

  /*
    Give up ownership of m_ptr, so as not to release them when
    this object is destroyed, to be called when the two member is shallow
    assigned to another geometry object.
   */
  virtual void donate_data() {
    set_ownmem(false);
    set_nbytes(0);
    m_ptr = nullptr;
  }

 protected:
  /**
    In a polygon usable by boost geometry, the m_ptr points to the outer ring
    object, and m_inn_rings points to the inner rings, thus the polygon's data
    isn't stored in a single WKB. Users should call
    @c Gis_polygon::to_wkb_unparsed() before getting the polygon's wkb data,
    @c Gis_polygon::to_wkb_unparsed() will form a single WKB for the polygon
    and refer to it with m_ptr, and release the outer ring object
    and the inner rings objects, and such an polygon isn't usable by BG any
    more, it's exactly what we got with
    @c Geometry::create_from_wkt / @c Geometry::create_from_wkt.
   */
  bool polygon_is_wkb_form() const {
    return m_flags.props & POLYGON_IN_WKB_FORM;
  }

  void polygon_is_wkb_form(bool b) {
    if (b)
      m_flags.props |= POLYGON_IN_WKB_FORM;
    else
      m_flags.props &= ~POLYGON_IN_WKB_FORM;
  }

  /**
    If call Gis_wkb_vector<T>::resize() to add a component to X, the
    geometry may have a geometry not stored inside the WKB buffer of X, hence
    X has out of line component. For such an X, user should call
    Gis_wkb_vector<T>::reassemble() before fetching its WKB data.
   */
  bool has_out_of_line_components() const {
    return m_flags.props & HAS_OUT_OF_LINE_COMPONENTS;
  }

  void has_out_of_line_components(bool b) {
    if (b)
      m_flags.props |= HAS_OUT_OF_LINE_COMPONENTS;
    else
      m_flags.props &= ~HAS_OUT_OF_LINE_COMPONENTS;
  }

  void clear_wkb_data();
  virtual void shallow_push(const Geometry *) { DBUG_ASSERT(false); }

 protected:
  /**
    The topmost (root) geometry object, whose m_ptr is the 1st byte of a
    wkb memory buffer. other geometry objects hold m_ptr which points
    inside somewhere in the memory buffer. when updating a geometry object,
    need to ask m_owner to reallocate memory if needed for new data.
   */
  Geometry *m_owner;

  /**
    Pointer to the geometry's wkb data's 1st byte, right after its
    wkb header if any.
    If the geometry is wkb_polygon, this field is a
    Gis_polygon_ring* pointer, pointing to the outer ring. Outer ring's wkb data
    is in the same wkb buffer as the inner rings, so we can get the wkb header
    from the outer ring like ((Geometry*)m_ptr)->get_ptr().
   */
  void *m_ptr;

 private:
  /// Flags and meta information about this object.
  /// Make it mutable to modify some flags in const member functions.
  mutable Flags_t m_flags;

  /// Srid of this object.
  gis::srid_t m_srid;

 public:
  Flags_t get_flags() const { return m_flags; }

  void set_flags(const Flags_t &flags) { m_flags = flags; }
};

inline Geometry::wkbByteOrder get_byte_order(const void *p0) {
  const char *p = static_cast<const char *>(p0);

  if (!(*p == 0 || *p == 1)) return Geometry::wkb_invalid;
  return *p == 0 ? Geometry::wkb_xdr : Geometry::wkb_ndr;
}

inline void set_byte_order(void *p0, Geometry::wkbByteOrder bo) {
  char *p = static_cast<char *>(p0);
  *p = (bo == Geometry::wkb_ndr ? 1 : 0);
}

/**
  Get wkbType value from WKB, the WKB is always little endian, so need
  platform specific conversion.
  @param p0 WKB geometry type field address.
  @return geometry type.
 */
inline Geometry::wkbType get_wkb_geotype(const void *p0) {
  const char *p = static_cast<const char *>(p0);
  uint32 gt = uint4korr(p);
  DBUG_ASSERT(Geometry::is_valid_geotype(gt));
  return static_cast<Geometry::wkbType>(gt);
}

/*
  Functions to write a GEOMETRY or WKB header into a piece of allocated and
  big enough raw memory or into a String object with enough reserved memory,
  and optionally append the object count right after the header.
 */
inline char *write_wkb_header(void *p0, Geometry::wkbType geotype) {
  char *p = static_cast<char *>(p0);
  *p = static_cast<char>(Geometry::wkb_ndr);
  p++;
  int4store(p, static_cast<uint32>(geotype));
  return p + 4;
}

inline char *write_wkb_header(void *p0, Geometry::wkbType geotype,
                              uint32 obj_count) {
  char *p = static_cast<char *>(p0);
  p = write_wkb_header(p, geotype);
  int4store(p, obj_count);
  return p + 4;
}

inline char *write_geometry_header(void *p0, gis::srid_t srid,
                                   Geometry::wkbType geotype) {
  char *p = static_cast<char *>(p0);
  int4store(p, srid);
  return write_wkb_header(p + 4, geotype);
}

inline char *write_geometry_header(void *p0, gis::srid_t srid,
                                   Geometry::wkbType geotype,
                                   uint32 obj_count) {
  char *p = static_cast<char *>(p0);
  int4store(p, srid);
  return write_wkb_header(p + 4, geotype, obj_count);
}

inline void write_wkb_header(String *str, Geometry::wkbType geotype) {
  q_append(static_cast<char>(Geometry::wkb_ndr), str);
  q_append(static_cast<uint32>(geotype), str);
}

inline void write_wkb_header(String *str, Geometry::wkbType geotype,
                             uint32 obj_count) {
  write_wkb_header(str, geotype);
  q_append(obj_count, str);
}

inline void write_geometry_header(String *str, gis::srid_t srid,
                                  Geometry::wkbType geotype) {
  q_append(srid, str);
  write_wkb_header(str, geotype);
}

inline void write_geometry_header(String *str, gis::srid_t srid,
                                  Geometry::wkbType geotype, uint32 obj_count) {
  write_geometry_header(str, srid, geotype);
  q_append(obj_count, str);
}

/***************************** Point *******************************/

class Gis_point : public Geometry {
 public:
  uint32 get_data_size() const override;
  /**
    Initialize from a partial WKT string (everything following "POINT").

    @param trs Input stream
    @param wkb Output string
    @param parens Whether parentheses are expected around the
    coordinates.
    @retval true Error
    @retval false Success
  */
  bool init_from_wkt(Gis_read_stream *trs, String *wkb, const bool parens);
  bool init_from_wkt(Gis_read_stream *trs, String *wkb) override {
    return init_from_wkt(trs, wkb, true);
  }
  uint init_from_wkb(THD *thd, const char *wkb, uint len, wkbByteOrder bo,
                     String *res) override;
  bool get_data_as_wkt(String *txt, wkb_parser *wkb) const override;
  bool get_mbr(MBR *mbr, wkb_parser *wkb) const override;

  int get_xy(point_xy *p) const {
    wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
    return wkb.scan_xy(p);
  }
  int get_x(double *x) const override {
    wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
    return wkb.scan_coord(x);
  }
  int get_y(double *y) const override {
    wkb_parser wkb(get_cptr(), get_cptr() + get_nbytes());
    return wkb.skip_coord() || wkb.scan_coord(y);
  }
  uint32 feature_dimension() const override { return 0; }
  const Class_info *get_class_info() const override;
  bool reverse_coordinates() override;
  bool validate_coordinate_range(double srs_angular_unit,
                                 bool *long_out_of_range,
                                 bool *lat_out_of_range,
                                 double *out_of_range_value) override;

  /************* Boost Geometry Adapter Interface *************/

  typedef Gis_point self;
  typedef Geometry base;

  explicit Gis_point(bool is_bg_adapter = true)
      : Geometry(nullptr, 0, Flags_t(wkb_point, 0), default_srid) {
    set_ownmem(false);
    set_bg_adapter(is_bg_adapter);
  }

  /// @brief Default constructor, no initialization.
  Gis_point(const void *ptr, size_t nbytes, const Flags_t &flags,
            gis::srid_t srid)
      : Geometry(ptr, nbytes, flags, srid) {
    set_geotype(wkb_point);
    DBUG_ASSERT(
        (ptr != nullptr && get_nbytes() == SIZEOF_STORED_DOUBLE * GEOM_DIM) ||
        (ptr == nullptr && get_nbytes() == 0));
    set_ownmem(false);
    set_bg_adapter(true);
  }

  Gis_point(const self &pt);

  ~Gis_point() override {}

  Gis_point &operator=(const Gis_point &rhs);

  void set_ptr(void *ptr, size_t len);

  /// @brief Get a coordinate
  /// @tparam K coordinate to get
  /// @return the coordinate
  template <std::size_t K>
  double get() const {
    DBUG_ASSERT(K < static_cast<size_t>(get_dimension()) &&
                ((m_ptr != nullptr &&
                  get_nbytes() == SIZEOF_STORED_DOUBLE * GEOM_DIM) ||
                 (m_ptr == nullptr && get_nbytes() == 0)));

    set_bg_adapter(true);
    const char *p = static_cast<char *>(m_ptr) + K * SIZEOF_STORED_DOUBLE;

    /*
      Boost Geometry may use a point that is only default constructed that
      has not specified with any meaningful value, and in such a case the
      default value are expected to be all zeros.
     */
    if (m_ptr == nullptr) return 0;

    return float8get(p);
  }

  /// @brief Set a coordinate
  /// @tparam K coordinate to set
  /// @param value value to set
  // Deep assignment, not only allow assigning to a point owning its memory,
  // but also a point not own memory, since points are of same size.
  template <std::size_t K>
  void set(double const &value) {
    /* Allow assigning to others' memory. */
    DBUG_ASSERT((m_ptr != nullptr && K < static_cast<size_t>(get_dimension()) &&
                 get_nbytes() == SIZEOF_STORED_DOUBLE * GEOM_DIM) ||
                (!get_ownmem() && get_nbytes() == 0 && m_ptr == nullptr));
    set_bg_adapter(true);
    if (m_ptr == nullptr) {
      m_ptr = gis_wkb_fixed_alloc(SIZEOF_STORED_DOUBLE * GEOM_DIM);
      if (m_ptr == nullptr) {
        set_ownmem(false);
        set_nbytes(0);
        return;
      }
      set_ownmem(true);
      set_nbytes(SIZEOF_STORED_DOUBLE * GEOM_DIM);
    }

    char *p = get_cptr() + K * SIZEOF_STORED_DOUBLE;
    float8store(p, value);
  }

  bool operator<(const Gis_point &pt) const {
    bool x = get<0>(), px = pt.get<0>();
    return x == px ? get<1>() < pt.get<1>() : x < px;
  }

  bool operator==(const Gis_point &pt) const {
    return (get<0>() == pt.get<0>() && get<1>() == pt.get<1>());
  }

  bool operator!=(const Gis_point &pt) const { return !(operator==(pt)); }
};

/******************************** Gis_wkb_vector **************************/

template <typename T>
class Gis_wkb_vector;

/// @ingroup iterators
/// @{
/// @defgroup Gis_wkb_vector_iterators Iterator classes for Gis_wkb_vector.
/// Gis_wkb_vector has two iterator classes --- Gis_wkb_vector_const_iterator
/// and Gis_wkb_vector_iterator. The differences
/// between the two classes are that the Gis_wkb_vector_const_iterator
/// can only be used to read its referenced value, so it is intended as
/// Gis_wkb_vector's const iterator; While the other class allows both read and
/// write access. If your access pattern is readonly, it is strongly
/// recommended that you use the const iterator because it is faster
/// and more efficient.
/// The two classes have identical behaviors to std::vector::const_iterator and
/// std::vector::iterator respectively.
//@{
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//
// Gis_wkb_vector_const_iterator class template definition
//
/// Gis_wkb_vector_const_iterator is const_iterator class for Gis_wkb_vector,
/// and base class of Gis_wkb_vector_iterator -- iterator class for
/// Gis_wkb_vector.
/// @tparam T Vector element type
template <typename T>
class Gis_wkb_vector_const_iterator {
 protected:
  typedef Gis_wkb_vector_const_iterator<T> self;
  typedef Gis_wkb_vector<T> owner_t;
  typedef ptrdiff_t index_type;

 public:
  ////////////////////////////////////////////////////////////////////
  //
  // Begin public type definitions.
  //
  typedef T value_type;
  typedef ptrdiff_t difference_type;
  typedef difference_type distance_type;
  typedef typename owner_t::size_type size_type;

  /// This is the return type for operator[].
  typedef value_type &reference;
  typedef value_type *pointer;
  // Use the STL tag, to ensure compatability with interal STL functions.
  //
  typedef std::random_access_iterator_tag iterator_category;
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  // Begin public constructors and destructor.
  /// @name Constructors and destroctor
  /// Do not construct iterators explictily using these constructors,
  /// but call Gis_wkb_vector::begin() const to get an valid iterator.
  /// @sa Gis_wkb_vector::begin() const
  //@{
  Gis_wkb_vector_const_iterator(const self &vi) {
    m_curidx = vi.m_curidx;
    m_owner = vi.m_owner;
  }

  Gis_wkb_vector_const_iterator() {
    m_curidx = -1;
    m_owner = nullptr;
  }

  Gis_wkb_vector_const_iterator(index_type idx, const owner_t *owner) {
    m_curidx = idx;
    m_owner = const_cast<owner_t *>(owner);
  }

  ~Gis_wkb_vector_const_iterator() {}
  //@}

  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  //
  // Begin functions that compare iterator positions.
  //
  /// @name Iterator comparison operators
  /// The way to compare two iterators is to compare the index values
  /// of the two elements they point to. The iterator sitting on an
  /// element with less index is regarded to be smaller. And the invalid
  /// iterator sitting after last element is greater than any other
  /// iterators, because it is assumed to have an index equal to last
  /// element's index plus one; The invalid iterator sitting before first
  /// element is less than any other iterators because it is assumed to
  /// have an index -1.
  //@{
  /// @brief Equality comparison operator.
  ///
  /// Invalid iterators are equal; Valid iterators
  /// sitting on the same key/data pair equal; Otherwise not equal.
  /// @param itr The iterator to compare against.
  /// @return True if this iterator equals to itr; False otherwise.
  bool operator==(const self &itr) const {
    DBUG_ASSERT(m_owner == itr.m_owner);
    return m_curidx == itr.m_curidx;
  }

  /// @brief Unequal compare, identical to !operator(==itr)
  /// @param itr The iterator to compare against.
  /// @return False if this iterator equals to itr; True otherwise.
  bool operator!=(const self &itr) const { return !(*this == itr); }

  // The end() iterator is largest. If both are end() iterator return false.
  /// @brief Less than comparison operator.
  /// @param itr The iterator to compare against.
  /// @return True if this iterator is less than itr.
  bool operator<(const self &itr) const {
    DBUG_ASSERT(m_owner == itr.m_owner);
    return m_curidx < itr.m_curidx;
  }

  /// @brief Less equal comparison operator.
  /// @param itr The iterator to compare against.
  /// @return True if this iterator is less than or equal to itr.
  bool operator<=(const self &itr) const { return !(this->operator>(itr)); }

  /// @brief Greater equal comparison operator.
  /// @param itr The iterator to compare against.
  /// @return True if this iterator is greater than or equal to itr.
  bool operator>=(const self &itr) const { return !(this->operator<(itr)); }

  // The end() iterator is largest. If both are end() iterator return false.
  /// @brief Greater comparison operator.
  /// @param itr The iterator to compare against.
  /// @return True if this iterator is greater than itr.
  bool operator>(const self &itr) const {
    DBUG_ASSERT(m_owner == itr.m_owner);
    return m_curidx > itr.m_curidx;
  }
  //@} // vctitr_cmp
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  //
  // Begin functions that shift the iterator position.
  //
  /// @name Iterator movement operators.
  /// When we talk about iterator movement, we think the
  /// container is a uni-directional range, represented by [begin, end),
  /// and this is true no matter we are using iterators or reverse
  /// iterators. When an iterator is moved closer to "begin", we say it
  /// is moved backward, otherwise we say it is moved forward.
  //@{
  /// @brief Pre-increment.
  ///
  /// Move the iterator one element forward, so that
  /// the element it sits on has a bigger index.
  /// Use ++iter rather than iter++ where possible to avoid two useless
  /// iterator copy constructions.
  /// @return This iterator after incremented.
  self &operator++() {
    move_by(*this, 1, false);
    return *this;
  }

  /// @brief Post-increment.
  /// Move the iterator one element forward, so that
  /// the element it sits on has a bigger index.
  /// Use ++iter rather than iter++ where possible to avoid two useless
  /// iterator copy constructions.
  /// @return A new iterator not incremented.
  self operator++(int) {
    self itr(*this);
    move_by(*this, 1, false);

    return itr;
  }

  /// @brief Pre-decrement.
  /// Move the iterator one element backward, so
  /// that the element it  sits on has a smaller index.
  /// Use --iter rather than iter-- where possible to avoid two useless
  /// iterator copy constructions.
  /// @return This iterator after decremented.
  self &operator--() {
    move_by(*this, 1, true);
    return *this;
  }

  /// @brief Post-decrement.
  ///
  /// Move the iterator one element backward, so
  /// that the element it  sits on has a smaller index.
  /// Use --iter rather than iter-- where possible to avoid two useless
  /// iterator copy constructions.
  /// @return A new iterator not decremented.
  self operator--(int) {
    self itr = *this;
    move_by(*this, 1, true);
    return itr;
  }

  /// @brief Assignment operator.
  ///
  /// This iterator will point to the same key/data
  /// pair as itr, and have the same configurations as itr.
  /// @param itr The right value of the assignment.
  /// @return This iterator's reference.
  const self &operator=(const self &itr) {
    m_curidx = itr.m_curidx;
    m_owner = itr.m_owner;
    return itr;
  }

  /// Iterator movement operator.
  /// Return another iterator by moving this iterator forward by n
  /// elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move backward by |n| element.
  /// @return The new iterator at new position.
  self operator+(difference_type n) const {
    self itr(*this);
    move_by(itr, n, false);
    return itr;
  }

  /// @brief Move this iterator forward by n elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move backward by |n| element.
  /// @return Reference to this iterator at new position.
  const self &operator+=(difference_type n) {
    move_by(*this, n, false);
    return *this;
  }

  /// @brief Iterator movement operator.
  ///
  /// Return another iterator by moving this iterator backward by n
  /// elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move forward by |n| element.
  /// @return The new iterator at new position.
  self operator-(difference_type n) const {
    self itr(*this);
    move_by(itr, n, true);

    return itr;
  }

  /// @brief Move this iterator backward by n elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move forward by |n| element.
  /// @return Reference to this iterator at new position.
  const self &operator-=(difference_type n) {
    move_by(*this, n, true);
    return *this;
  }
  //@} //itr_movement

  /// @brief Iterator distance operator.
  ///
  /// Return the index difference of this iterator and itr, so if this
  /// iterator sits on an element with a smaller index, this call will
  /// return a negative number.
  /// @param itr The other iterator to substract. itr can be the invalid
  /// iterator after last element or before first element, their index
  /// will be regarded as last element's index + 1 and -1 respectively.
  /// @return The index difference.
  difference_type operator-(const self &itr) const {
    DBUG_ASSERT(m_owner == itr.m_owner);
    return (m_curidx - itr.m_curidx);
  }

  ////////////////////////////////////////////////////////////////////
  //
  // Begin functions that retrieve values from the iterator.
  //
  /// @name Functions that retrieve values from the iterator.
  //@{
  /// @brief Dereference operator.
  ///
  /// Return the reference to the cached data element.
  /// The returned value can only be used to read its referenced
  /// element.
  /// @return The reference to the element this iterator points to.
  reference operator*() const {
    DBUG_ASSERT(this->m_owner != nullptr && this->m_curidx >= 0 &&
                this->m_curidx <
                    static_cast<index_type>(this->m_owner->size()));
    return (*m_owner)[m_curidx];
  }

  /// @brief Arrow operator.
  ///
  /// Return the pointer to the cached data element.
  /// The returned value can only be used to read its referenced
  /// element.
  /// @return The address of the referenced object.
  pointer operator->() const {
    DBUG_ASSERT(this->m_owner != NULL && this->m_curidx >= 0 &&
                this->m_curidx <
                    static_cast<index_type>(this->m_owner->size()));
    return &(*m_owner)[m_curidx];
  }

  /// @brief Iterator index operator.
  ///
  /// @param offset The offset of target element relative to this iterator.
  /// @return Return the reference of the element which is at
  /// position *this + offset.
  /// The returned value can only be used to read its referenced
  /// element.
  reference operator[](difference_type offset) const {
    self itr = *this;
    move_by(itr, offset, false);

    DBUG_ASSERT(itr.m_owner != NULL && itr.m_curidx >= 0 &&
                itr.m_curidx < static_cast<index_type>(itr.m_owner->size()));
    return (*m_owner)[itr.m_curidx];
  }
  //@}
  ////////////////////////////////////////////////////////////////////

 protected:
  // The 'back' parameter indicates whether to decrease or
  // increase the index when moving. The default is to decrease.
  //
  void move_by(self &itr, difference_type n, bool back) const {
    if (back) n = -n;

    index_type newidx = itr.m_curidx + n;
    size_t sz = 0;

    if (newidx < 0)
      newidx = -1;
    else if (newidx >= static_cast<index_type>((sz = m_owner->size())))
      newidx = sz;

    itr.m_curidx = newidx;
  }

 protected:
  /// Current element's index, starting from 0.
  index_type m_curidx;
  /// The owner container of this iteraotr.
  owner_t *m_owner;
};  // Gis_wkb_vector_const_iterator<>

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//
// Gis_wkb_vector_iterator class template definition
/// This class is the iterator class for Gis_wkb_vector, its instances can
/// be used to mutate their referenced data element.
/// @tparam T Vector element type
//
template <class T>
class Gis_wkb_vector_iterator : public Gis_wkb_vector_const_iterator<T> {
 protected:
  typedef Gis_wkb_vector_iterator<T> self;
  typedef Gis_wkb_vector_const_iterator<T> base;
  typedef Gis_wkb_vector<T> owner_t;

 public:
  typedef ptrdiff_t index_type;
  typedef T value_type;
  typedef ptrdiff_t difference_type;
  typedef difference_type distance_type;
  typedef value_type &reference;
  typedef value_type *pointer;
  // Use the STL tag, to ensure compatability with interal STL functions.
  typedef std::random_access_iterator_tag iterator_category;

  ////////////////////////////////////////////////////////////////////
  /// Begin public constructors and destructor.
  //
  /// @name Constructors and destructor
  /// Do not construct iterators explictily using these constructors,
  /// but call Gis_wkb_vector::begin to get an valid iterator.
  /// @sa Gis_wkb_vector::begin
  //@{
  Gis_wkb_vector_iterator(const self &vi) : base(vi) {}

  Gis_wkb_vector_iterator() : base() {}

  Gis_wkb_vector_iterator(const base &obj) : base(obj) {}

  Gis_wkb_vector_iterator(index_type idx, const owner_t *owner)
      : base(idx, owner) {}

  ~Gis_wkb_vector_iterator() {}
  //@}

  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  //
  /// Begin functions that shift the iterator position.
  //
  /// These functions are identical to those defined in
  /// Gis_wkb_vector_const_iterator, but we have to redefine them here because
  /// the "self" have different definitions.
  //
  /// @name Iterator movement operators.
  /// These functions have identical behaviors and semantics as those of
  /// Gis_wkb_vector_const_iterator, so please refer to equivalent in that
  /// class.
  //@{
  /// @brief Pre-increment.
  /// @return This iterator after incremented.
  /// @sa Gis_wkb_vector_const_iterator::operator++()
  self &operator++() {
    this->move_by(*this, 1, false);
    return *this;
  }

  /// @brief Post-increment.
  /// @return A new iterator not incremented.
  /// @sa Gis_wkb_vector_const_iterator::operator++(int)
  self operator++(int) {
    self itr(*this);
    this->move_by(*this, 1, false);

    return itr;
  }

  /// @brief Pre-decrement.
  /// @return This iterator after decremented.
  /// @sa Gis_wkb_vector_const_iterator::operator--()
  self &operator--() {
    this->move_by(*this, 1, true);
    return *this;
  }

  /// @brief Post-decrement.
  /// @return A new iterator not decremented.
  /// @sa Gis_wkb_vector_const_iterator::operator--(int)
  self operator--(int) {
    self itr = *this;
    this->move_by(*this, 1, true);
    return itr;
  }

  /// @brief Assignment operator.
  ///
  /// This iterator will point to the same key/data
  /// pair as itr, and have the same configurations as itr.
  /// @param itr The right value of the assignment.
  /// @return This iterator's reference.
  const self &operator=(const self &itr) {
    base::operator=(itr);

    return itr;
  }

  /// @brief Iterator movement operator.
  ///
  /// Return another iterator by moving this iterator backward by n
  /// elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move forward by |n| element.
  /// @return The new iterator at new position.
  /// @sa Gis_wkb_vector_const_iterator::operator+(difference_type n) const
  self operator+(difference_type n) const {
    self itr(*this);
    this->move_by(itr, n, false);
    return itr;
  }

  /// @brief Move this iterator backward by n elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move forward by |n| element.
  /// @return Reference to this iterator at new position.
  /// @sa Gis_wkb_vector_const_iterator::operator+=(difference_type n)
  const self &operator+=(difference_type n) {
    this->move_by(*this, n, false);
    return *this;
  }

  /// @brief Iterator movement operator.
  ///
  /// Return another iterator by moving this iterator forward by n
  /// elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move backward by |n| element.
  /// @return The new iterator at new position.
  /// @sa Gis_wkb_vector_const_iterator::operator-(difference_type n) const
  self operator-(difference_type n) const {
    self itr(*this);
    this->move_by(itr, n, true);

    return itr;
  }

  /// @brief Move this iterator forward by n elements.
  /// @param n The amount and direction of movement. If negative, will
  /// move backward by |n| element.
  /// @return Reference to this iterator at new position.
  /// @sa Gis_wkb_vector_const_iterator::operator-=(difference_type n)
  const self &operator-=(difference_type n) {
    this->move_by(*this, n, true);
    return *this;
  }
  //@} // itr_movement

  /// @brief Iterator distance operator.
  ///
  /// Return the index difference of this iterator and itr, so if this
  /// iterator sits on an element with a smaller index, this call will
  /// return a negative number.
  /// @param itr The other iterator to substract. itr can be the invalid
  /// iterator after last element or before first element, their index
  /// will be regarded as last element's index + 1 and -1 respectively.
  /// @return The index difference.
  /// @sa Gis_wkb_vector_const_iterator::operator-(const self &itr) const
  difference_type operator-(const self &itr) const {
    return base::operator-(itr);
  }
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  //
  // Begin functions that retrieve values from the iterator.
  //
  /// @name Functions that retrieve values from the iterator.
  //@{
  /// @brief Dereference operator.
  ///
  /// Return the reference to the cached data element
  /// The returned value can be used to read or update its referenced
  /// element.
  /// @return The reference to the element this iterator points to.
  reference operator*() const {
    DBUG_ASSERT(this->m_owner != nullptr && this->m_curidx >= 0 &&
                this->m_curidx <
                    static_cast<index_type>(this->m_owner->size()));
    return (*this->m_owner)[this->m_curidx];
  }

  /// @brief Arrow operator.
  ///
  /// Return the pointer to the cached data element
  /// The returned value can be used to read or update its referenced
  /// element.
  /// @return The address of the referenced object.
  pointer operator->() const {
    DBUG_ASSERT(this->m_owner != nullptr && this->m_curidx >= 0 &&
                this->m_curidx <
                    static_cast<index_type>(this->m_owner->size()));
    return &(*this->m_owner)[this->m_curidx];
  }

  /// @brief Iterator index operator.
  ///
  /// @param offset The offset of target element relative to this iterator.
  /// @return Return the element which is at position *this + offset.
  /// The returned value can be used to read or update its referenced
  /// element.
  reference operator[](difference_type offset) const {
    self itr = *this;
    this->move_by(itr, offset, false);
    DBUG_ASSERT(itr.m_owner != NULL && itr.m_curidx >= 0 &&
                itr.m_curidx < static_cast<index_type>(this->m_owner->size()));
    return (*this->m_owner)[itr.m_curidx];
  }
  //@} // funcs_val
  ////////////////////////////////////////////////////////////////////

};  // Gis_wkb_vector_iterator
//@} // Gis_wkb_vector_iterators
//@} // iterators

// These operators make "n + itr" expressions valid. Without it, you can only
// use "itr + n"
template <typename T>
Gis_wkb_vector_const_iterator<T> operator+(
    typename Gis_wkb_vector_const_iterator<T>::difference_type n,
    const Gis_wkb_vector_const_iterator<T> &itr) {
  Gis_wkb_vector_const_iterator<T> itr2 = itr;

  itr2 += n;
  return itr2;
}

template <typename T>
Gis_wkb_vector_iterator<T> operator+(
    typename Gis_wkb_vector_iterator<T>::difference_type n,
    const Gis_wkb_vector_iterator<T> &itr) {
  Gis_wkb_vector_iterator<T> itr2 = itr;

  itr2 += n;
  return itr2;
}

void *get_packed_ptr(const Geometry *geo, size_t *pnbytes);
const char *get_packed_ptr(Geometry *geo);
bool polygon_is_packed(Geometry *plgn, Geometry *mplgn);
void own_rings(Geometry *geo);
void parse_wkb_data(Geometry *geom, const char *p, size_t num_geoms = 0);

/**
   Geometry vector class.
   @tparam T Vector element type.
 */
template <typename T>
class Geometry_vector : public Inplace_vector<T> {
  typedef Inplace_vector<T> base;

 public:
  Geometry_vector() : base(PSI_INSTRUMENT_ME) {}
};

/// @ingroup containers
//@{
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
/// Gis_wkb_vector class template definition
/// @tparam T Vector element type
//
template <typename T>
class Gis_wkb_vector : public Geometry {
 private:
  typedef Gis_wkb_vector<T> self;
  typedef ptrdiff_t index_type;
  typedef Geometry base;

 public:
  typedef T value_type;
  typedef Gis_wkb_vector_const_iterator<T> const_iterator;
  typedef Gis_wkb_vector_iterator<T> iterator;
  typedef size_t size_type;
  typedef const T *const_pointer;
  typedef const T &const_reference;
  typedef T *pointer;
  typedef T &reference;
  typedef ptrdiff_t difference_type;

  typedef Geometry_vector<T> Geo_vector;

 private:
  /**
    The geometry vector of this geometry object's components, each of which
    is an object of Geometry or its children classes where appropriate.
   */
  Geo_vector *m_geo_vect;

 public:
  /////////////////////////////////////////////////////////////////////
  // Begin functions that create iterators.
  /// @name Iterator functions.
  //@{
  iterator begin() {
    set_bg_adapter(true);
    iterator itr(m_geo_vect ? 0 : -1, this);
    return itr;
  }

  /// @brief Create a const iterator.
  ///
  /// The created iterator can only be used to read its referenced
  /// data element. Can only be called when using a const reference to
  /// the contaienr object.
  const_iterator begin() const {
    set_bg_adapter(true);
    const_iterator itr(m_geo_vect ? 0 : -1, this);
    return itr;
  }

  /// @brief Create an open boundary iterator.
  /// @return Returns an invalid iterator denoting the position after
  /// the last valid element of the container.
  iterator end() {
    iterator itr(m_geo_vect ? m_geo_vect->size() : -1, this);
    return itr;
  }

  /// @brief Create an open boundary iterator.
  /// @return Returns an invalid const iterator denoting the position
  /// after the last valid element of the container.
  const_iterator end() const {
    const_iterator itr(m_geo_vect ? m_geo_vect->size() : -1, this);
    return itr;
  }

  //@} // iterator_funcs
  /////////////////////////////////////////////////////////////////////

  /// @brief Get container size.
  /// @return Return the number of elements in this container.
  size_type size() const {
    set_bg_adapter(true);
    return m_geo_vect ? m_geo_vect->size() : 0;
  }

  bool empty() const { return size() == 0; }

  const_reference back() const {
    set_bg_adapter(true);
    /*
      Carefully crafted to avoid invoking any copy constructor using pointer
      cast. Also true for the two operator[] member functions below.
     */
    const Geometry *p = &(get_geo_vect()->back());
    return *((const T *)p);
  }

  reference back() {
    set_bg_adapter(true);
    /*
      Carefully crafted to avoid invoking any copy constructor using pointer
      cast. Also true for the two operator[] member functions below.
     */
    Geometry *p = &(get_geo_vect()->back());
    return *((T *)p);
  }

  const_reference operator[](index_type i) const {
    DBUG_ASSERT(!(i < 0 || i >= (index_type)size()));
    set_bg_adapter(true);

    const Geometry *p = &((*m_geo_vect)[i]);
    return *((const T *)p);
  }

  reference operator[](index_type i) {
    DBUG_ASSERT(!(i < 0 || i >= (index_type)size()));
    set_bg_adapter(true);

    Geometry *p = &((*m_geo_vect)[i]);
    return *((T *)p);
  }

  Gis_wkb_vector(const void *ptr, size_t nbytes, const Geometry::Flags_t &flags,
                 gis::srid_t srid, bool is_bg_adapter = true);
  Gis_wkb_vector(const self &v);

  Gis_wkb_vector() : Geometry() { m_geo_vect = nullptr; }

  ~Gis_wkb_vector() override {
    /*
      See ~Geometry() for why we do try-catch like this.

      Note that although ~Inplace_vector() calls std::vector member functions,
      all of them have no-throw guarantees, so this function won't throw any
      exception now. We do so nonetheless for potential mis-use of exceptions
      in futher code.
    */
#if !defined(DBUG_OFF)
    try {
#endif
      if (!is_bg_adapter()) return;
      if (m_geo_vect != nullptr) clear_wkb_data();
#if !defined(DBUG_OFF)
    } catch (...) {
      // Should never throw exceptions in destructor.
      DBUG_ASSERT(false);
    }
#endif
  }

  void clear_wkb_data() {
    delete m_geo_vect;
    m_geo_vect = nullptr;
  }

  self &operator=(const self &rhs);

  // SUPPRESS_UBSAN Wrong downcast. FIXME
  void shallow_push(const Geometry *g) override SUPPRESS_UBSAN;

  Geo_vector *get_geo_vect(bool create_if_null = false) {
    if (m_geo_vect == nullptr && create_if_null) m_geo_vect = new Geo_vector;
    return m_geo_vect;
  }

  Geo_vector *get_geo_vect() const { return m_geo_vect; }

  void set_geo_vect(Geo_vector *ptr) { m_geo_vect = ptr; }

  /*
    Give up ownership of m_ptr and m_geo_vect, so as not to release them when
    this object is destroyed, to be called when the two member is shallow
    assigned to another geometry object.
   */
  void donate_data() override {
    set_ownmem(false);
    set_nbytes(0);
    m_ptr = nullptr;
    m_geo_vect = nullptr;
  }

  void set_ptr(void *ptr, size_t len);
  void clear();
  size_t get_nbytes_free() const;
  size_t current_size() const;
  void push_back(const T &val);
  void resize(size_t sz);
  void reassemble();
  bool reverse_coordinates() override {
    DBUG_ASSERT(false);
    return true;
  }
  bool validate_coordinate_range(double, bool *, bool *, double *) override {
    DBUG_ASSERT(false); /* purecov: inspected */
    return true;        /* purecov: inspected */
  }

 private:
  typedef Gis_wkb_vector<Gis_point> Linestring;
  typedef Gis_wkb_vector<Linestring> Multi_linestrings;

};  // Gis_wkb_vector

//@} //

/***************************** LineString *******************************/

class Gis_line_string : public Gis_wkb_vector<Gis_point> {
  // Maximum number of points in LineString that can fit into String
  static const uint32 max_n_points =
      (uint32)(UINT_MAX32 - WKB_HEADER_SIZE - 4 /* n_points */) /
      POINT_DATA_SIZE;

 public:
  uint32 get_data_size() const override;
  bool init_from_wkt(Gis_read_stream *trs, String *wkb) override;
  uint init_from_wkb(THD *thd, const char *wkb, uint len, wkbByteOrder bo,
                     String *res) override;
  bool get_data_as_wkt(String *txt, wkb_parser *wkb) const override;
  bool get_mbr(MBR *mbr, wkb_parser *wkb) const override;
  int geom_length(double *len) const override;
  int is_closed(int *closed) const override;
  int num_points(uint32 *n_points) const override;
  int start_point(String *point) const override;
  int end_point(String *point) const override;
  int point_n(uint32 n, String *result) const override;
  uint32 feature_dimension() const override { return 1; }
  const Class_info *get_class_info() const override;
  bool reverse_coordinates() override;
  bool validate_coordinate_range(double srs_angular_unit,
                                 bool *long_out_of_range,
                                 bool *lat_out_of_range,
                                 double *out_of_range_value) override;

  /**** Boost Geometry Adapter Interface ******/

  typedef Gis_wkb_vector<Gis_point> base_type;
  typedef Gis_line_string self;

  explicit Gis_line_string(bool is_bg_adapter = true)
      : base_type(nullptr, 0, Flags_t(wkb_linestring, 0), default_srid,
                  is_bg_adapter) {}

  Gis_line_string(const void *wkb, size_t len, const Flags_t &flags,
                  gis::srid_t srid)
      : base_type(wkb, len, flags, srid, true) {
    set_geotype(wkb_linestring);
  }

  Gis_line_string(const self &ls) : base_type(ls) {}

  Gis_line_string &operator=(const Gis_line_string &) = default;
};

/*
  We have to use such an independent class in order to meet Ring Concept of
  Boost Geometry --- there must be a specialization of traits::tag defining
  ring_tag as type.
  If directly use Gis_line_string, we would have defined that tag twice.
*/
class Gis_polygon_ring : public Gis_wkb_vector<Gis_point> {
 public:
  typedef Gis_wkb_vector<Gis_point> base;
  typedef Gis_polygon_ring self;

  virtual ~Gis_polygon_ring() {}
  Gis_polygon_ring(const void *wkb, size_t nbytes, const Flags_t &flags,
                   gis::srid_t srid)
      : base(wkb, nbytes, flags, srid, true) {
    set_geotype(wkb_linestring);
  }

  // Coordinate data type, closed-ness and direction will never change, thus no
  // need for the template version of copy constructor.
  Gis_polygon_ring(const self &r) : base(r) {}

  Gis_polygon_ring &operator=(const Gis_polygon_ring &) = default;

  Gis_polygon_ring()
      : base(nullptr, 0, Flags_t(Geometry::wkb_linestring, 0), default_srid,
             true) {}

  bool set_ring_order(bool want_ccw);
};

/***************************** Polygon *******************************/

// For internal use only, only convert types, don't create rings.
inline Gis_polygon_ring *outer_ring(const Geometry *g) {
  DBUG_ASSERT(g->get_geotype() == Geometry::wkb_polygon);
  Gis_polygon_ring *out = static_cast<Gis_polygon_ring *>(g->get_ptr());

  return out;
}

class Gis_polygon : public Geometry {
 public:
  uint32 get_data_size() const override;
  bool init_from_wkt(Gis_read_stream *trs, String *wkb) override;
  uint init_from_wkb(THD *thd, const char *wkb, uint len, wkbByteOrder bo,
                     String *res) override;
  bool get_data_as_wkt(String *txt, wkb_parser *wkb) const override;
  bool get_mbr(MBR *mbr, wkb_parser *wkb) const override;
  int exterior_ring(String *result) const override;
  int num_interior_ring(uint32 *n_int_rings) const override;
  int interior_ring_n(uint32 num, String *result) const override;
  uint32 feature_dimension() const override { return 2; }
  const Class_info *get_class_info() const override;
  bool reverse_coordinates() override;
  bool validate_coordinate_range(double srs_angular_unit,
                                 bool *long_out_of_range,
                                 bool *lat_out_of_range,
                                 double *out_of_range_value) override;

  /**** Boost Geometry Adapter Interface ******/
  typedef Gis_polygon self;
  typedef Gis_polygon_ring ring_type;
  typedef Gis_wkb_vector<ring_type> inner_container_type;

  ring_type &outer() const {
    DBUG_ASSERT(!polygon_is_wkb_form());
    set_bg_adapter(true);
    // Create outer ring if none, although read only, calller may just want
    // to traverse the outer ring if any.
    if (this->m_ptr == nullptr) const_cast<self *>(this)->make_rings();

    return *(outer_ring(this));
  }

  inner_container_type &inners() const {
    DBUG_ASSERT(!polygon_is_wkb_form());
    set_bg_adapter(true);
    // Create inner rings if none, although read only, calller may just want
    // to traverse the inner rings if any.
    if (m_inn_rings == nullptr) const_cast<self *>(this)->make_rings();

    return *m_inn_rings;
  }

  /// Clears outer and inner rings.
  void clear() {
    set_bg_adapter(true);
    outer_ring(this)->clear();
    if (m_inn_rings) m_inn_rings->clear();
  }

  Gis_polygon(const void *wkb, size_t nbytes, const Flags_t &flags,
              gis::srid_t srid);

  /*
    We can't require boost geometry use the 'polygon' in any particular way,
    so we have to default to true.
  */
  explicit Gis_polygon(bool isbgadapter = true)
      : Geometry(nullptr, 0, Flags_t(Geometry::wkb_polygon, 0), default_srid) {
    m_inn_rings = nullptr;
    set_bg_adapter(isbgadapter);
  }

  Gis_polygon(const self &r);
  Gis_polygon &operator=(const Gis_polygon &rhs);
  ~Gis_polygon() override;

  void to_wkb_unparsed();
  void set_ptr(void *ptr, size_t len);

  /*
    Give up ownership of m_ptr and m_inn_rings, so as not to release them when
    this object is destroyed, to be called when the two member is shallow
    assigned to another geometry object.
   */
  void donate_data() override {
    set_ownmem(false);
    set_nbytes(0);
    m_ptr = nullptr;
    m_inn_rings = nullptr;
  }

  bool set_polygon_ring_order();

  // SUPPRESS_UBSAN Wrong downcast. FIXME
  inner_container_type *inner_rings() const SUPPRESS_UBSAN {
    return m_inn_rings;
  }

  // SUPPRESS_UBSAN Wrong downcast. FIXME
  void set_inner_rings(inner_container_type *inns) SUPPRESS_UBSAN {
    m_inn_rings = inns;
  }

 private:
  inner_container_type *m_inn_rings;

  void make_rings();
};

/***************************** MultiPoint *******************************/

class Gis_multi_point : public Gis_wkb_vector<Gis_point> {
  // Maximum number of points in MultiPoint that can fit into String
  static const uint32 max_n_points =
      (uint32)(UINT_MAX32 - WKB_HEADER_SIZE - 4 /* n_points */) /
      (WKB_HEADER_SIZE + POINT_DATA_SIZE);

 public:
  uint32 get_data_size() const override;
  bool init_from_wkt(Gis_read_stream *trs, String *wkb) override;
  uint init_from_wkb(THD *thd, const char *wkb, uint len, wkbByteOrder bo,
                     String *res) override;
  bool get_data_as_wkt(String *txt, wkb_parser *wkb) const override;
  bool get_mbr(MBR *mbr, wkb_parser *wkb) const override;
  int num_geometries(uint32 *num) const override;
  int geometry_n(uint32 num, String *result) const override;
  uint32 feature_dimension() const override { return 0; }
  const Class_info *get_class_info() const override;
  bool reverse_coordinates() override;
  bool validate_coordinate_range(double srs_angular_unit,
                                 bool *long_out_of_range,
                                 bool *lat_out_of_range,
                                 double *out_of_range_value) override;

  /**** Boost Geometry Adapter Interface ******/

  typedef Gis_wkb_vector<Gis_point> base_type;
  typedef Gis_multi_point self;

  explicit Gis_multi_point(bool is_bg_adapter = true)
      : base_type(nullptr, 0, Flags_t(wkb_multipoint, 0), default_srid,
                  is_bg_adapter) {}

  Gis_multi_point(const void *ptr, size_t nbytes, const Flags_t &flags,
                  gis::srid_t srid)
      : base_type(ptr, nbytes, flags, srid, true) {
    set_geotype(wkb_multipoint);
  }

  Gis_multi_point(const self &mpts) : base_type(mpts) {}
};

/***************************** MultiLineString *******************************/

class Gis_multi_line_string : public Gis_wkb_vector<Gis_line_string> {
 public:
  uint32 get_data_size() const override;
  bool init_from_wkt(Gis_read_stream *trs, String *wkb) override;
  uint init_from_wkb(THD *thd, const char *wkb, uint len, wkbByteOrder bo,
                     String *res) override;
  bool get_data_as_wkt(String *txt, wkb_parser *wkb) const override;
  bool get_mbr(MBR *mbr, wkb_parser *wkb) const override;
  int num_geometries(uint32 *num) const override;
  int geometry_n(uint32 num, String *result) const override;
  int geom_length(double *len) const override;
  int is_closed(int *closed) const override;
  uint32 feature_dimension() const override { return 1; }
  const Class_info *get_class_info() const override;
  bool reverse_coordinates() override;
  bool validate_coordinate_range(double srs_angular_unit,
                                 bool *long_out_of_range,
                                 bool *lat_out_of_range,
                                 double *out_of_range_value) override;

  /**** Boost Geometry Adapter Interface ******/

  typedef Gis_wkb_vector<Gis_line_string> base;
  typedef Gis_multi_line_string self;

  explicit Gis_multi_line_string(bool is_bg_adapter = true)
      : base(nullptr, 0, Flags_t(wkb_multilinestring, 0), default_srid,
             is_bg_adapter) {}

  Gis_multi_line_string(const void *ptr, size_t nbytes, const Flags_t &,
                        gis::srid_t srid)
      : base(ptr, nbytes, Flags_t(wkb_multilinestring, nbytes), srid, true) {
    set_geotype(wkb_multilinestring);
  }

  Gis_multi_line_string(const self &mls) : base(mls) {}
};

/***************************** MultiPolygon *******************************/

class Gis_multi_polygon : public Gis_wkb_vector<Gis_polygon> {
 public:
  uint32 get_data_size() const override;
  bool init_from_wkt(Gis_read_stream *trs, String *wkb) override;
  uint init_from_wkb(THD *thd, const char *wkb, uint len, wkbByteOrder bo,
                     String *res) override;
  bool get_data_as_wkt(String *txt, wkb_parser *wkb) const override;
  bool get_mbr(MBR *mbr, wkb_parser *wkb) const override;
  int num_geometries(uint32 *num) const override;
  int geometry_n(uint32 num, String *result) const override;
  uint32 feature_dimension() const override { return 2; }
  const Class_info *get_class_info() const override;
  bool reverse_coordinates() override;
  bool validate_coordinate_range(double srs_angular_unit,
                                 bool *long_out_of_range,
                                 bool *lat_out_of_range,
                                 double *out_of_range_value) override;

  /**** Boost Geometry Adapter Interface ******/
  typedef Gis_multi_polygon self;
  typedef Gis_wkb_vector<Gis_polygon> base;

  explicit Gis_multi_polygon(bool is_bg_adapter = true)
      : base(nullptr, 0, Flags_t(wkb_multipolygon, 0), default_srid,
             is_bg_adapter) {}

  Gis_multi_polygon(const void *ptr, size_t nbytes, const Flags_t &flags,
                    gis::srid_t srid)
      : base(ptr, nbytes, flags, srid, true) {
    set_geotype(wkb_multipolygon);
  }

  Gis_multi_polygon(const self &mpl) : base(mpl) {}
};

/*********************** GeometryCollection *******************************/
class Gis_geometry_collection : public Geometry {
 private:
  static Geometry *scan_header_and_create(wkb_parser *wkb,
                                          Geometry_buffer *buffer);

 public:
  Gis_geometry_collection()
      : Geometry(nullptr, 0, Flags_t(wkb_geometrycollection, 0), default_srid) {
    set_bg_adapter(false);
  }
  Gis_geometry_collection(Geometry *geo, String *gcbuf);
  Gis_geometry_collection(gis::srid_t srid, wkbType gtype, const String *gbuf,
                          String *gcbuf);
  bool append_geometry(const Geometry *geo, String *gcbuf);
  bool append_geometry(gis::srid_t srid, wkbType gtype, const String *gbuf,
                       String *gcbuf);
  uint32 get_data_size() const override;
  bool init_from_wkt(Gis_read_stream *trs, String *wkb) override;
  uint init_from_wkb(THD *thd, const char *wkb, uint len, wkbByteOrder bo,
                     String *res) override;
  bool get_data_as_wkt(String *txt, wkb_parser *wkb) const override;
  bool get_mbr(MBR *mbr, wkb_parser *wkb) const override;
  int num_geometries(uint32 *num) const override;
  int geometry_n(uint32 num, String *result) const override;
  bool dimension(uint32 *dim, wkb_parser *wkb) const override;
  uint32 feature_dimension() const override {
    DBUG_ASSERT(0);
    return 0;
  }
  bool reverse_coordinates() override;
  bool validate_coordinate_range(double srs_angular_unit,
                                 bool *long_out_of_range,
                                 bool *lat_out_of_range,
                                 double *out_of_range_value) override;
  const Class_info *get_class_info() const override;
};

/**
  Gis_polygon objects and Gis_wkb_vector<> objects are of same size, and
  Gis_point and Geometry objects are smaller. They are always allocated
  inside a Geometry_buffer object, unless used as boost geometry adapter,
  in which case the object may simply placed on stack or new'ed on heap.
 */
struct Geometry_buffer {
  alignas(Gis_polygon) char data[sizeof(Gis_polygon)];
};

class WKB_scanner_event_handler {
 public:
  virtual ~WKB_scanner_event_handler() {}

  /**
    Notified when scanner sees the start of a geometry WKB.
    @param bo byte order of the WKB.
    @param geotype geometry type of the WKB;
    @param wkb WKB byte string, the first byte after the WKB header if any.
    @param len NO. of bytes of the WKB byte string starting from wkb.
               There can be many geometries in the [wkb, wkb+len) buffer.
    @param has_hdr whether there is a WKB header right before 'wkb' in the
                   byte string.
   */
  virtual void on_wkb_start(Geometry::wkbByteOrder bo,
                            Geometry::wkbType geotype, const void *wkb,
                            uint32 len, bool has_hdr) = 0;

  /**
    Notified when scanner sees the end of a geometry WKB.
    @param wkb the position of the first byte after the WKB byte string which
               the scanner just scanned.
   */
  virtual void on_wkb_end(const void *wkb) = 0;

  /*
    Called after each on_wkb_start/end call, if returns false, wkb_scanner
    will stop scanning.
   */
  virtual bool continue_scan() const { return true; }
};

const char *wkb_scanner(THD *thd, const char *wkb, uint32 *len, uint32 geotype,
                        bool has_hdr, WKB_scanner_event_handler *handler);
#endif
