/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/base/zend/zend_php_config.h>
#include <runtime/ext/ext_curl.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_imagesprite.h>
#include <runtime/ext/ext_string.h>
#include <runtime/ext/ext_url.h>
#include <list>
#include <vector>
#include <queue>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace ImageSprite {

// PHP extension gd.c
#define PHP_GDIMG_TYPE_GIF      1
#define PHP_GDIMG_TYPE_PNG      2
#define PHP_GDIMG_TYPE_JPG      3
#define PHP_GDIMG_TYPE_WBM      4
#define PHP_GDIMG_TYPE_XBM      5
#define PHP_GDIMG_TYPE_XPM      6
#define PHP_GDIMG_CONVERT_WBM   7
#define PHP_GDIMG_TYPE_GD       8
#define PHP_GDIMG_TYPE_GD2      9
#define PHP_GDIMG_TYPE_GD2PART 10

// PHP extension STANDARD: image.c
// /* file type markers */
static const char php_sig_gif[3] = {'G', 'I', 'F'};
static const char php_sig_jpg[3] = {(char) 0xff, (char) 0xd8, (char) 0xff};
static const char php_sig_png[8] =
  {(char) 0x89, (char) 0x50, (char) 0x4e, (char) 0x47,
  (char) 0x0d, (char) 0x0a, (char) 0x1a, (char) 0x0a};
static const char php_sig_gd2[3] = {'g', 'd', '2'};

void ev_req_complete(struct evhttp_request *req, void *obj) {
  assert(obj);
  ((ImageFromHTTP*)obj)->completed();
}

bool image_height_comparator(const Image* lhs, const Image* rhs) {
  return lhs->m_effheight < rhs->m_effheight;
}

bool image_max_dim_comparator(const Image* lhs, const Image* rhs) {
  int l = (lhs->m_width > lhs->m_height) ? lhs->m_width : lhs->m_height;
  int r = (rhs->m_width > rhs->m_height) ? rhs->m_width : rhs->m_height;
  return l < r;
}

bool image_area_comparator(const Image* lhs, const Image* rhs) {
  return lhs->m_effarea < rhs->m_effarea;
}

bool image_path_comparator(const Image* lhs, const Image* rhs) {
  return lhs->m_path < rhs->m_path;
}

class BlockHeightComparator {
  public:
    bool operator()(const Block* lhs, const Block* rhs) {
      return lhs->m_height < rhs->m_height;
    }
};

class BlockAreaComparator {
  public:
    bool operator()(const Block* lhs, const Block* rhs) {
      return lhs->m_area < rhs->m_area;
    }
};

static int _php_image_type (char data[8]) {
  if (data == NULL) {
    return -1;
  }

  if (!memcmp(data, php_sig_gd2, 3)) {
    return PHP_GDIMG_TYPE_GD2;
  } else if (!memcmp(data, php_sig_jpg, 3)) {
    return PHP_GDIMG_TYPE_JPG;
  } else if (!memcmp(data, php_sig_png, 3)) {
    if (!memcmp(data, php_sig_png, 8)) {
      return PHP_GDIMG_TYPE_PNG;
    }
  } else if (!memcmp(data, php_sig_gif, 3)) {
    return PHP_GDIMG_TYPE_GIF;
  }
  return -1;
}

void ResourceGroupEv::add(evhttp_connection* conn) {
  m_connections += 1;
  evhttp_connection_set_base(conn, m_base);
}

void ResourceGroupEv::complete(evhttp_connection* conn) {
  m_connections -= 1;
  if (m_connections <= 0) {
    event_base_loopbreak(m_base);
  }
}

void ResourceGroupEv::setTimeout(int milliseconds) {
  int seconds;
  if (milliseconds > 0) {
    seconds = milliseconds / 1000;
    milliseconds = milliseconds % 1000;
  } else {
    seconds = 10;
    milliseconds = 0;
  }
  struct timeval timeout_struct;
  timeout_struct.tv_sec = seconds;
  timeout_struct.tv_usec = milliseconds * 1000;
  int ret = event_base_loopexit(m_base, &timeout_struct);
  if (ret != 0) {
    // TODO: Handle timeout setting failure (how?)
  }
}

void Image::loadDims(bool block /* = false */) {
  assert(!m_error);
  if (m_width <= 0 || m_height <= 0) {
    loadImage(block);
    if (block && !m_error) {
      assert(m_image != NULL);
      m_width = m_image->sx;
      m_height = m_image->sy;
    }
  }
}

void Image::reset() {
  if (m_image) {
    gdImageDestroy(m_image);
    m_image = NULL;
  }
  m_width = -1;
  m_height = -1;
  m_area = -1;
  m_error = false;
  m_error_string = null_string;
}

void Image::setError(String err) {
  m_error = true;
  m_error_string = err;
  String sprite_err = String("Error in image '") + m_path + "': " + err;
  m_sprite->m_img_errors.append(sprite_err);
}

void Image::load_data_to_gd(int size, const void* data) {
  assert(data);
  assert(!m_error);
  assert(m_image == NULL);
  if (size < 8) {
    setError("Invalid image file");
    return;
  }

  char sig[8];
  memcpy(sig, data, 8);
  int imtype = _php_image_type(sig);
  gdImagePtr img = NULL;
  switch(imtype) {
    case PHP_GDIMG_TYPE_JPG:
  #ifdef HAVE_GD_JPG
      img = gdImageCreateFromJpegPtr(size, (void*) data);
  #else
      setError("No JPEG support");
      return;
  #endif
      break;

    case PHP_GDIMG_TYPE_PNG:
  #ifdef HAVE_GD_PNG
      img = gdImageCreateFromPngPtr(size, (void*) data);
  #else
      setError("No PNG support");
      return;
  #endif
      break;

    case PHP_GDIMG_TYPE_GIF:
  #ifdef HAVE_GD_GIF_READ
      img = gdImageCreateFromGifPtr(size, (void*) data);
  #else
      setError("No GIF support");
      return;
  #endif
      break;

    case PHP_GDIMG_TYPE_GD2:
  #ifdef HAVE_GD_GD2
      img = gdImageCreateFromGd2Ptr(size, (void*) data);
  #else
      setError("No GD2 support");
      return;
  #endif
      break;

    default:
      setError("Data format not supported");
      return;
  }

  if (img) {
    m_image = img;
    if (m_width != img->sx || m_height != img->sy) {
      m_width = img->sx;
      m_height = img->sy;
      m_area = img->sx * img->sy;
      m_sprite->m_current = false;
    }
  } else {
    setError("Error converting data to image.");
    m_width = -1;
    m_height = -1;
    m_area = -1;
    m_sprite->m_current = false;
  }
}

void Image::setOptions(Array options) {
  if (options.exists("padding_top")) {
    int32 v = options["padding_top"].toInt32();
    if (v < 0) {
      m_padding[IMAGESPRITE_PAD_TOP] = 0;
    } else {
      m_padding[IMAGESPRITE_PAD_TOP] = v;
    }
  }

  if (options.exists("padding_right")) {
    int32 v = options["padding_right"].toInt32();
    if (v < 0) {
      m_padding[IMAGESPRITE_PAD_RIGHT] = 0;
    } else {
      m_padding[IMAGESPRITE_PAD_RIGHT] = v;
    }
  }

  if (options.exists("padding_bottom")) {
    int32 v = options["padding_bottom"].toInt32();
    if (v < 0) {
      m_padding[IMAGESPRITE_PAD_BOTTOM] = 0;
    } else {
      m_padding[IMAGESPRITE_PAD_BOTTOM] = v;
    }
  }

  if (options.exists("padding_left")) {
    int32 v = options["padding_left"].toInt32();
    if (v < 0) {
      m_padding[IMAGESPRITE_PAD_LEFT] = 0;
    } else {
      m_padding[IMAGESPRITE_PAD_LEFT] = v;
    }
  }

  if (options.exists("width") && options.exists("height")) {
    m_width = options["width"].toInt32();
    m_height = options["height"].toInt32();
    m_area = m_width * m_height;
  }

  if (options.exists("flush_left")) {
    bool v = options["flush_left"].toBoolean();
    m_flush[IMAGESPRITE_FLUSH_LEFT] = v;
  }

  if (options.exists("flush_right")) {
    bool v = options["flush_right"].toBoolean();
    m_flush[IMAGESPRITE_FLUSH_RIGHT] = v;
  }
}

void ImageFromFile::loadImage(bool block /* = false */) {
  assert(!m_error);
  if (m_image != NULL) {
    return;
  }
  // Don't do anything unless we are blocking
  if (block) {
    Variant contents = f_file_get_contents(m_path);
    if (contents.isString()) {
      String data = contents.toString();
      load_data_to_gd(data.size(), data.c_str());
    } else {
      setError("Cannot locate file");
    }
  }
}

void ImageFromString::loadImage(bool block /* = false */) {
  assert(!m_error);
  if (m_image != NULL) {
    return;
  }
  // Don't do anything unless we are blocking
  if (block) {
    load_data_to_gd(m_data.size(), m_data.c_str());
  }
}

ImageFromHTTP::ImageFromHTTP(String path, c_ImageSprite* sprite, int timeout)
    : Image(path, sprite) {
  m_ev_group = NULL;
  m_host = null_string;
  m_url = null_string;
  m_query = "";
  m_port = 80;
  m_conn = NULL;
  m_request = NULL;
  Variant parts = f_parse_url(path);
  if (parts.isArray()) {
    for (ArrayIter iter(parts.toArray()); iter; ++iter) {
      String key = iter.first().toString();
      String val = iter.second().toString();
      if (key == "scheme") {
        if (val != "http") {
          setError("Unsupported URL scheme");
          break;
        }
      } else if (key == "host") {
        m_host = val;
      } else if (key == "port") {
        m_port = val.toInt32();
      } else if (key == "path") {
        m_url = val;
      } else if (key == "query") {
        m_query += "?";
        m_query += val;
      }
    }

    // Now verify we got everything we need
    if (same(m_host, null_string)) {
      setError("No host specified");
    } else if (same(m_url, null_string)) {
      setError("No url path specified");
    } else if (m_sprite->m_rsrc_groups["ev"] == NULL) {
      // Get/initialize the event group with the base
      m_ev_group = new ResourceGroupEv(m_sprite);
      m_sprite->m_rsrc_groups["ev"] = m_ev_group;
    } else {
      m_ev_group = (ResourceGroupEv*) m_sprite->m_rsrc_groups["ev"];
    }

    if (m_ev_group != NULL) {
      m_ev_group->setTimeout(timeout);
    }
  }
}

void ImageFromHTTP::loadImage(bool block /* = false */) {
  assert(!m_error);
  if (m_image != NULL) {
    return;
  }

  if (m_conn == NULL) {
    m_conn = evhttp_connection_new(m_host.c_str(), m_port);
    m_ev_group->add(m_conn);
    m_request = evhttp_request_new(ev_req_complete, this);

    if (m_port == 80) {
      evhttp_add_header(m_request->output_headers, "Host", m_host.c_str());
    } else {
      std::string ss = m_host.data();
      ss += ":";
      ss += boost::lexical_cast<std::string>(m_port);
      evhttp_add_header(m_request->output_headers, "Host", ss.c_str());
    }

    int ret = evhttp_make_request(
      m_conn,
      m_request,
      EVHTTP_REQ_GET,
      (m_url + m_query).c_str());

    if (ret) {
      setError("http request failed");
    }
  }

  if (block) {
    event_base_dispatch(m_ev_group->m_base);
  }
}

void ImageFromHTTP::reset() {
  if (m_conn != NULL) {
    evhttp_connection_free(m_conn);
    m_conn = NULL;
  }

  m_request = NULL;
  Image::reset();
}

void ImageFromHTTP::completed() {
  assert(m_request);
  assert(m_request->input_buffer);
  assert(m_image == NULL);
  assert(!m_error);

  int code = m_request->response_code;
  if (code >= 200 && code <= 300) {
    // Looks like we got a real response
    load_data_to_gd(
      EVBUFFER_LENGTH(m_request->input_buffer),
      EVBUFFER_DATA(m_request->input_buffer));
  } else {
    setError(String("Invalid Response code: ") + String((int64)code));
  }
  m_ev_group->complete(m_conn);
  evhttp_connection_free(m_conn);
  m_conn = NULL;
  m_request = NULL;
}

} // Imagesprite

// PHP accessible classes/functions

c_ImageSprite::c_ImageSprite() {
  m_image_string_buffer = null_string;
  m_image = NULL;
  m_current = false;
  m_width = 0;
  m_height = 0;
  m_mapping = Array::Create();
  m_img_errors = Array::Create();
  m_sprite_errors = Array::Create();
}

c_ImageSprite::~c_ImageSprite() {
}

void c_ImageSprite::t___construct() {
}

Object c_ImageSprite::t_addfile(CStrRef file,
                                CArrRef options /* = null */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::addfile);
  m_current = false;

  ImageSprite::Image *entry = new ImageSprite::ImageFromFile(file, this);
  entry->setOptions(options);
  if (m_image_data[file.c_str()] != NULL) {
    delete m_image_data[file.c_str()];
  }
  m_image_data[file.c_str()] = entry;

  return this;
}

Object c_ImageSprite::t_addstring(CStrRef id, CStrRef data,
                                  CArrRef options /* = null */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::addstring);
  m_current = false;

  ImageSprite::Image *entry = new ImageSprite::ImageFromString(id, data, this);
  entry->setOptions(options);
  if (m_image_data[id.c_str()] != NULL) {
    delete m_image_data[id.c_str()];
  }
  m_image_data[id.c_str()] = entry;

  return this;
}

Object c_ImageSprite::t_addurl(CStrRef url, int timeout_ms /* = 0 */,
                               CArrRef options /* = null */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::addurl);
  m_current = false;

  ImageSprite::Image *entry = new ImageSprite::ImageFromHTTP(
    url,
    this,
    timeout_ms);

  entry->setOptions(options);
  if (m_image_data[url.c_str()] != NULL) {
    delete m_image_data[url.c_str()];
  }
  m_image_data[url.c_str()] = entry;

  return this;
}

Object c_ImageSprite::t_clear(CVarRef files /* = null */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::clear);
  if (same(files, null)) {
    // Clear them all, might as well __destruct
    t___destruct();
  } else if (files.isArray()) {
    for (ArrayIter iter(files.toArray()); iter; ++iter) {
      if (iter.second().isString()) {
        t_clear(iter.second());
      }
    }
  } else if (files.isString()) {
    String path(files.toString());
    if (m_image_data.find(path.c_str()) == m_image_data.end()) {
      return this;
    }
    m_current = false;
    delete m_image_data[path.c_str()];
    m_image_data.erase(path.c_str());
  } else {
    // TODO: Wrong type passed to function
  }

  return this;
}

Object c_ImageSprite::t_loaddims(bool block /* = false */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::loaddims);
  if (same(m_current, true)) {
    return this;
  }

  if (block) {
    // This is so http requests can be batched, etc
    t_loaddims(false);
  }

  hphp_string_map<ImageSprite::Image*>::iterator iter;
  hphp_string_map<ImageSprite::Image*>::iterator end;

  iter = m_image_data.begin();
  end = m_image_data.end();

  for(; iter != end; iter++) {
    ImageSprite::Image *val = iter->second;
    if (val == NULL) {
      continue;
    } else if (!val->m_error) {
      val->loadDims(block);
    }
  }

  return this;
}

Object c_ImageSprite::t_loadimages(bool block /* = false */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::loadimages);
  if (same(m_current, true)) {
    return this;
  }

  if (block) {
    // This is so http requests can be batched, etc
    t_loadimages(false);
  }

  hphp_string_map<ImageSprite::Image*>::iterator iter;
  hphp_string_map<ImageSprite::Image*>::iterator end;

  iter = m_image_data.begin();
  end = m_image_data.end();

  for(; iter != end; iter++) {
    ImageSprite::Image *val = iter->second;
    if (val == NULL) {
      continue;
    } else if (!val->m_error) {
      val->loadImage(block);
    }
  }

  return this;
}

#define ImageList std::list<ImageSprite::Image*>

#define BlockHeightMinHeap std::priority_queue< \
          ImageSprite::Block*, \
          std::vector<ImageSprite::Block*>, \
          ImageSprite::BlockHeightComparator>

#define BlockAreaMinHeap std::priority_queue< \
          ImageSprite::Block*, \
          std::vector<ImageSprite::Block*>, \
          ImageSprite::BlockAreaComparator>

void c_ImageSprite::map() {
  if (same(m_current, true)) {
    return;
  }

  m_mapping.removeAll();

  delete m_image;
  m_image = NULL;

  ImageList images;

  /**
   * We're putting all of the elements in the map into a list here because then
   * we can sort them and guarantee the iteration order. Failure to guarantee
   * iteration order may result in images of the same size being placed in
   * opposite positions, and the wrong one being shown (if the css and output
   * are generated on separate machines)
   **/

  hphp_string_map<ImageSprite::Image*>::iterator map_iter;
  hphp_string_map<ImageSprite::Image*>::iterator map_end;

  map_iter = m_image_data.begin();
  map_end = m_image_data.end();

  for(; map_iter != map_end; map_iter++) {
    ImageSprite::Image *img = map_iter->second;
    if (!img->m_error) {
      images.push_front(img);
    }
  }

  if (images.empty()) {
    // Get out before it's too late
    return;
  }

  // Here be dragons. Thou art forewarned

  images.sort(ImageSprite::image_path_comparator);

  int height = 0;
  int width = 0;

  ImageList left_flush;
  ImageList right_flush;
  ImageList no_flush;
  BlockHeightMinHeap right_heap;
  BlockAreaMinHeap general_heap;
  int abs_width = 0;

  ImageList::iterator iter = images.begin();
  ImageList::iterator end = images.end();

  // Loop through images
  // sort them by whether they are left flush, right flush, or no flush
  for(; iter != end; iter++) {
    ImageSprite::Image *img = *iter;

    if (img == NULL || img->m_error) {
      continue;
    }

    img->m_effwidth = img->m_padding[IMAGESPRITE_PAD_LEFT] +
                      img->m_width +
                      img->m_padding[IMAGESPRITE_PAD_RIGHT];
    img->m_effheight = img->m_padding[IMAGESPRITE_PAD_TOP] +
                      img->m_height +
                      img->m_padding[IMAGESPRITE_PAD_BOTTOM];
    img->m_effarea = img->m_effwidth * img->m_effheight;

    if (img->m_flush[IMAGESPRITE_FLUSH_LEFT]) {
      left_flush.push_front(img);

      if (img->m_flush[IMAGESPRITE_FLUSH_RIGHT]) {
        if (abs_width && img->m_effwidth != abs_width) {
          m_img_errors.append(
            "Two images specified as flush both left and right, "
            "but have different widths");
          return;
        } else {
          abs_width = img->m_effwidth;
        }
      }
    } else if (img->m_flush[IMAGESPRITE_FLUSH_RIGHT]) {
      right_flush.push_front(img);
    } else {
      no_flush.push_front(img);
    }

    if (width < img->m_effwidth) {
      width = img->m_effwidth;
      if (abs_width && width > abs_width) {
        m_img_errors.append(
          "Widest image is wider that an image specified "
          "as flush both left and right");
        return;
      }
    }
  }

  right_flush.sort(ImageSprite::image_height_comparator);
  no_flush.sort(ImageSprite::image_max_dim_comparator);

  iter = left_flush.begin();
  end = left_flush.end();

  // Loop through the left flush images and stack them.
  // Put the blocks in the flush right heap
  for(; iter != end; iter++) {
    ImageSprite::Image *img = *iter;

    img->m_x = img->m_padding[IMAGESPRITE_PAD_LEFT];
    img->m_y = img->m_padding[IMAGESPRITE_PAD_TOP] + height;

    if (img->m_effwidth < width) {
      int x = img->m_effwidth;
      int y = height;
      ImageSprite::Block *block = new ImageSprite::Block(
        x,
        y,
        width - x,
        img->m_effheight);
      right_heap.push(block);
    }

    height += img->m_effheight;
  }

  // Loop the right images and place them
  while(!right_heap.empty() && !right_flush.empty()) {
    ImageSprite::Block* block = right_heap.top();
    right_heap.pop();

    bool fit = false;
    ImageList::iterator best_iter;

    iter = right_flush.begin();
    end = right_flush.end();
    for(; iter != end; iter++) {
      ImageSprite::Image *img = *iter;

      if (img->m_effheight > block->m_height) {
        break;
      }

      if (img->m_effwidth > block->m_width) {
        continue;
      }

      fit = true;
      best_iter = iter;
    }

    if (fit) {
      ImageSprite::Image *img = *best_iter;
      img->m_x = width - img->m_effwidth + img->m_padding[IMAGESPRITE_PAD_LEFT];
      img->m_y = block->m_y + img->m_padding[IMAGESPRITE_PAD_TOP];

      if (img->m_effheight != block->m_height ||
          img->m_effwidth != block->m_width) {
        int vcut_b = img->m_effwidth * (block->m_height - img->m_effheight);
        int vcut_l = block->m_height * (block->m_width - img->m_effwidth);
        int hcut_b = block->m_width * (block->m_height - img->m_effheight);
        int hcut_l = img->m_effheight * (block->m_width - img->m_effwidth);
        int vcut_d = (vcut_b > vcut_l) ? vcut_b - vcut_l : vcut_l - vcut_b;
        int hcut_d = (hcut_b > hcut_l) ? hcut_b - hcut_l : hcut_l - hcut_b;
        if (hcut_d > vcut_d) {
          // Horizontal cut is better than a vertical cut
          if (img->m_effwidth != block->m_width) {
            ImageSprite::Block* n_block = new ImageSprite::Block(
              block->m_x,
              block->m_y,
              (block->m_width - img->m_effwidth),
              img->m_effheight);
            general_heap.push(n_block);
          }
          if (img->m_effheight != block->m_height) {
            ImageSprite::Block* n_block = new ImageSprite::Block(
              block->m_x,
              block->m_y + img->m_effheight,
              block->m_width,
              block->m_height - img->m_effheight);
            right_heap.push(n_block);
          }
        } else {
          // Vertical cut is better than a horizontal cut
          if (img->m_effwidth != block->m_width) {
            ImageSprite::Block* n_block = new ImageSprite::Block(
              block->m_x,
              block->m_y,
              (block->m_width - img->m_effwidth),
              block->m_height);
            general_heap.push(n_block);
          }
          if (img->m_effheight != block->m_height) {
            ImageSprite::Block* n_block = new ImageSprite::Block(
              block->m_x + block->m_width - img->m_effwidth,
              block->m_y + img->m_effheight,
              img->m_effwidth,
              block->m_height - img->m_effheight);
            right_heap.push(n_block);
          }
        }
      }

      right_flush.erase(best_iter);
      delete block;
    } else {
      // Could be used by the no flush images
      general_heap.push(block);
    }

  }

  iter = right_flush.begin();
  end = right_flush.end();

  // Loop through the remaining right flush images
  // put them below what we've already placed
  for(; iter != end; iter++) {
    ImageSprite::Image *img = *iter;

    img->m_x = width - img->m_effwidth + img->m_padding[IMAGESPRITE_PAD_LEFT];
    img->m_y = height + img->m_padding[IMAGESPRITE_PAD_TOP];

    if (img->m_effwidth < width) {
      ImageSprite::Block *block = new ImageSprite::Block(
        0,
        height,
        width - img->m_effwidth,
        img->m_effheight);
      general_heap.push(block);
    }

    height += img->m_effheight;
  }

  // Now for the no flush images
  while(!no_flush.empty()) {
    while(!general_heap.empty() && !no_flush.empty()) {
      ImageSprite::Block* block = general_heap.top();
      general_heap.pop();

      bool fit = false;
      ImageList::iterator best_iter;

      iter = no_flush.begin();
      end = no_flush.end();
      for(; iter != end; iter++) {
        ImageSprite::Image *img = *iter;

        if (img->m_effarea > block->m_area) {
          break;
        }

        if (img->m_effheight > block->m_height ||
            img->m_effwidth > block->m_width) {
          continue;
        }

        fit = true;
        best_iter = iter;
      }

      if (fit) {
        ImageSprite::Image *img = *best_iter;

        img->m_x = block->m_x + img->m_padding[IMAGESPRITE_PAD_LEFT];
        img->m_y = block->m_y + img->m_padding[IMAGESPRITE_PAD_TOP];

        if (img->m_effheight != block->m_height ||
            img->m_effwidth != block->m_width) {
          int vcut_b = img->m_effwidth * (block->m_height - img->m_effheight);
          int vcut_l = block->m_height * (block->m_width - img->m_effwidth);
          int hcut_b = block->m_width * (block->m_height - img->m_effheight);
          int hcut_l = img->m_effheight * (block->m_width - img->m_effwidth);
          int vcut_d = (vcut_b > vcut_l) ? vcut_b - vcut_l : vcut_l - vcut_b;
          int hcut_d = (hcut_b > hcut_l) ? hcut_b - hcut_l : hcut_l - hcut_b;
          if (hcut_d > vcut_d) {
            // Horizontal cut is better than a vertical cut
            if (img->m_effwidth != block->m_width) {
              ImageSprite::Block* n_block = new ImageSprite::Block(
                block->m_x + img->m_effwidth,
                block->m_y,
                (block->m_width - img->m_effwidth),
                img->m_effheight);
              general_heap.push(n_block);
            }
            if (img->m_effheight != block->m_height) {
              ImageSprite::Block* n_block = new ImageSprite::Block(
                block->m_x,
                block->m_y + img->m_effheight,
                block->m_width,
                block->m_height - img->m_effheight);
              general_heap.push(n_block);
            }
          } else {
            // Vertical cut is better than a horizontal cut
            if (img->m_effwidth != block->m_width) {
              ImageSprite::Block* n_block = new ImageSprite::Block(
                block->m_x + img->m_effwidth,
                block->m_y,
                (block->m_width - img->m_effwidth),
                block->m_height);
              general_heap.push(n_block);
            }
            if (img->m_effheight != block->m_height) {
              ImageSprite::Block* n_block = new ImageSprite::Block(
                block->m_x,
                block->m_y + img->m_effheight,
                img->m_effwidth,
                block->m_height - img->m_effheight);
              general_heap.push(n_block);
            }
          }
        }

        no_flush.erase(best_iter);
      }

      delete block;
    }

    while (general_heap.empty() && !no_flush.empty()) {
      // We ran out of blocks, but there are still images left. Put it below!
      // The largest is at the end of the list - lets use it
      ImageSprite::Image *img = no_flush.back();
      no_flush.pop_back();

      img->m_x = img->m_padding[IMAGESPRITE_PAD_LEFT];
      img->m_y = height + img->m_padding[IMAGESPRITE_PAD_TOP];

      if (img->m_effwidth < width) {
        ImageSprite::Block* n_block = new ImageSprite::Block(
          img->m_effwidth,
          height,
          width - img->m_effwidth,
          img->m_effheight);
        general_heap.push(n_block);
      }

      height += img->m_effheight;
    }
  }

  // Everything's been placed, now to put it in a nice array to return
  iter = images.begin();
  end = images.end();

  Array image_map = Array::Create();

  for(; iter != end; iter++) {
    ImageSprite::Image *img = *iter;
    Array map;
    map.set("x", img->m_x);
    map.set("y", img->m_y);
    map.set("width", img->m_width);
    map.set("height", img->m_height);
    map.set("id", f_crc32(img->m_path));
    map.set("padding_top", img->m_padding[IMAGESPRITE_PAD_TOP]);
    map.set("padding_right", img->m_padding[IMAGESPRITE_PAD_RIGHT]);
    map.set("padding_bottom", img->m_padding[IMAGESPRITE_PAD_BOTTOM]);
    map.set("padding_left", img->m_padding[IMAGESPRITE_PAD_LEFT]);
    map.set("flush_left", img->m_flush[IMAGESPRITE_FLUSH_LEFT]);
    map.set("flush_right", img->m_flush[IMAGESPRITE_FLUSH_RIGHT]);
    image_map.set(img->m_path, map);
  }

  m_width = width;
  m_height = height;

  m_mapping.set("images", image_map);
  m_mapping.set("width", width);
  m_mapping.set("height", height);

  m_current = true;
}

String c_ImageSprite::t_output(CStrRef output_file /* = null_string*/,
                               CStrRef format /* = "png" */,
                               int32 quality /* = 75 */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::output);
  t_loadimages(true);
  map();

  if (m_width <= 0 || m_height <= 0) {
    // Danger Will Robinson! Danger!
    m_sprite_errors.append("Invalid sprite dimensions");
    return null_string;
  }

  if (m_image == NULL) {
    m_image = gdImageCreateTrueColor(m_width, m_height);
    gdImageSaveAlpha(m_image, 1);
    int transparent = gdImageColorAllocateAlpha(
      m_image, 255, 255, 255, 127);
    gdImageFill(m_image, 0, 0, transparent);

    hphp_string_map<ImageSprite::Image*>::iterator iter;
    hphp_string_map<ImageSprite::Image*>::iterator end;

    iter = m_image_data.begin();
    end = m_image_data.end();

    for(; iter != end; iter++) {
      ImageSprite::Image *img = iter->second;
      if (img == NULL || img->m_image == NULL) {
        continue;
      } else if (!img->m_error) {
        gdImageCopy(
          m_image,
          img->m_image,
          img->m_x,
          img->m_y,
          0,
          0,
          img->m_width,
          img->m_height);
      }
    }
  }

  String output_string = "Error";
  int mem_size = 0;
  void* mem_loc = NULL;

  String format_l= f_strtolower(format);

  if (same(format_l, "png")) {
    mem_loc = gdImagePngPtr(m_image, &mem_size);
  } else if (format_l == "jpg" || format_l == "jpeg") {
    if (quality <= 0 || quality > 100) {
      quality = 75;
    }
    mem_loc = gdImageJpegPtr(m_image, &mem_size, quality);
  } else if (same(format_l, "gif")) {
    mem_loc = gdImageGifPtr(m_image, &mem_size);
  } else {
    m_sprite_errors.append("Unsupported output format");
    return null_string;
  }

  if (mem_loc != NULL) {
    output_string = String((char*) mem_loc, mem_size, CopyString);
    gdFree(mem_loc);
  }

  if (output_file.empty()) {
    return output_string;
  } else {
    f_file_put_contents(output_file, output_string);
    return null_string;
  }
}

String c_ImageSprite::t_css(CStrRef css_namespace,
                            CStrRef sprite_file /* = null_string */,
                            CStrRef output_file /* = null_string */,
                            bool verbose /* = false */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::css);
  t_loaddims(true);
  map();

  String output = "";
  if (!sprite_file.empty()) {
    if (verbose) {
      output +=
        String("/* Image dimensions: ") +
        String(m_width) +
        "x" +
        String(m_height) +
        " */\n";
    }
    output += String(".") + css_namespace + "{";
    if (verbose) output += "\n  ";
    output += String("background-image: url('") + sprite_file + "');";
    if (verbose) output += "\n  ";
    output += "background-repeat: no-repeat";
    if (verbose) output += "\n";
    output += "}";
  }

  for (ArrayIter iter(m_mapping); iter; ++iter) {
    String path = iter.first().toString();
    Array attr = iter.second().toArray();

    if (verbose) {
      output += String("/* File: ") + path + " */\n";
    }

    output += String(".") + css_namespace + "#i" + attr["id"] + "{";
    if (verbose) output += "\n  ";

    output += "background-position:";
    if (more(attr["x"], 0)) {
      output += String("-") + attr["x"] + "px";
    } else {
      output += "0";
    }
    if (more(attr["y"], 0)) {
      output += String(" -") + attr["y"] + "px";
    } else {
      output += " 0";
    }
    output += ";";
    if (sprite_file.empty()) {
      if (verbose) output += "\n  ";
      output += "background-repeat: no-repeat;";
    }

    if (verbose) output += "\n  ";
    output += String("width:") + attr["width"] + "px;";
    if (verbose) output += "\n  ";
    output += String("height:") + attr["height"] + "px";
    if (verbose) output += "\n";
    output += "}";
    if (verbose) output += "\n";
  }

  if (output_file.empty()) {
    return output;
  } else {
    f_file_put_contents(output_file, output);
    return null_string;
  }
}

Array c_ImageSprite::t_geterrors() {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::geterrors);
  Array ret = Array::Create();
  ret.set("images", m_img_errors);
  ret.set("sprite", m_sprite_errors);
  return ret;
}

Array c_ImageSprite::t_mapping() {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::mapping);
  t_loaddims(true);
  map();

  return m_mapping;
}

Variant c_ImageSprite::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(ImageSprite, ImageSprite::__destruct);
  // Sic Trogdor on everything
  if (m_image != NULL) {
    delete m_image;
    m_image = NULL;
  }

  // Destroy all the imagesprite_Image objects
  hphp_string_map<ImageSprite::Image*>::iterator iter;
  hphp_string_map<ImageSprite::Image*>::iterator end;

  iter = m_image_data.begin();
  end = m_image_data.end();

  for(; iter != end; iter++) {
    ImageSprite::Image *val = iter->second;
    if (val != NULL) {
      delete val;
      // Cleared later
    }
  }

  m_image_data.clear();

  m_image_string_buffer = null_string;
  m_image = NULL;
  m_current = false;
  m_width = 0;
  m_height = 0;
  m_mapping = Array::Create();
  m_img_errors = Array::Create();
  m_sprite_errors = Array::Create();

  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
