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

#ifndef __EXT_IMAGESPRITE_INCLUDE_H__
#define __EXT_IMAGESPRITE_INCLUDE_H__

#include <runtime/base/base_includes.h>
#include <evhttp.h>
#include <gd.h>

namespace HPHP {

#define IMAGESPRITE_FLUSH_LEFT  0
#define IMAGESPRITE_FLUSH_RIGHT 1

#define IMAGESPRITE_PAD_TOP     0
#define IMAGESPRITE_PAD_LEFT    1
#define IMAGESPRITE_PAD_BOTTOM  2
#define IMAGESPRITE_PAD_RIGHT   3

class c_ImageSprite;

namespace ImageSprite {

class ResourceGroup {
  public:
    ResourceGroup(c_ImageSprite* sprite) {
      m_sprite = sprite;
    }
    virtual ~ResourceGroup() {
      m_sprite = NULL;
    }

  protected:
    c_ImageSprite* m_sprite;
};

class ResourceGroupEv : public ResourceGroup {
  public:
    ResourceGroupEv(c_ImageSprite* sprite) : ResourceGroup(sprite) {
      m_base = event_base_new();
      m_connections = 0;
    }

    ~ResourceGroupEv() {
      event_base_free(m_base);
      m_base = NULL;
    }

    void add(evhttp_connection* conn);

    void complete(evhttp_connection* conn);

    void setTimeout(int milliseconds);

    event_base* m_base;
    int m_connections;
};


class Image {
  public:
    Image(String path, c_ImageSprite* sprite) {
      m_sprite = sprite;
      m_path = path;
      m_image = NULL;
      m_width = -1;
      m_height = -1;
      m_area = -1;
      m_error = false;
      m_error_string = null_string;
      m_padding[IMAGESPRITE_PAD_TOP]       = 0;
      m_padding[IMAGESPRITE_PAD_RIGHT]     = 1;
      m_padding[IMAGESPRITE_PAD_BOTTOM]    = 1;
      m_padding[IMAGESPRITE_PAD_LEFT]      = 0;
      m_flush[IMAGESPRITE_FLUSH_LEFT]  = false;
      m_flush[IMAGESPRITE_FLUSH_RIGHT] = false;
    }

    ~Image() {
      reset();
    }

    virtual void loadImage(bool block = false) {
      // Implemented by the child
    }

    virtual void loadDims(bool block = false);

    virtual void reset();

    void setError(String err);

    void setOptions(Array options);

  protected:
    void load_data_to_gd(int size, const void* data);

  public:
    int m_x;
    int m_y;
    int m_area;

    c_ImageSprite* m_sprite;
    String m_path;

    gdImagePtr m_image;

    int m_width;
    int m_height;

    bool m_error;
    String m_error_string;

    int m_padding[4];
    bool m_flush[2];

    int m_effwidth;
    int m_effheight;
    int m_effarea;
};

class ImageFromFile : public Image {
  public:
    ImageFromFile(String path, c_ImageSprite* sprite)
      : Image(path, sprite) {
      // Do nothing
    }

    void loadImage(bool block = false);
};

class ImageFromString : public Image {
  public:
    ImageFromString(String id, String data, c_ImageSprite* sprite)
      : Image(id, sprite) {
      m_data = data;
    }

    void loadImage(bool block = false);

    private:
      String m_data;
};

class ImageFromHTTP : public Image {
  public:
    ImageFromHTTP(String path, c_ImageSprite* sprite, int timeout);

    void loadImage(bool block = false);

    void reset();
    void completed();

  protected:
    ResourceGroupEv* m_ev_group;
    String m_host;
    String m_url;
    String m_query;
    int m_port;
    evhttp_connection *m_conn;
    evhttp_request *m_request;
};

class Block {
  public:
    Block(int x, int y, int width, int height) {
      m_x = x;
      m_y = y;
      m_width = width;
      m_height = height;
      m_area = width * height;
    }
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    int m_area;
};

} // ImageSprite
} // HPHP
#endif // __EXT_IMAGESPRITE_INCLUDE_H__
