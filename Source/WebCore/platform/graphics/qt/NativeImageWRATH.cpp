
#include "NativeImageWRATH.h"
#include "FloatRect.h"
#include "IntRect.h"

#if USE(WRATH)

#include "WRATHNew.hpp"
#include "WRATHTripleBufferEnabler.hpp"

#include <QImage>

#include "WRATHImage.hpp"
#include "WRATHQTImageSupport.hpp"

#include <iostream>
#include <vector>
#include <sstream>

namespace {


class PyramidLevel
{
public:  
  PyramidLevel(void):
    m_size(0,0)
  {}
  
  
  void
  init(const ivec2 &psize)
  {
    m_size=psize;
    m_values.resize(array_size(m_size), 0);
  }

  void
  init(int w, int h)
  {
    init(ivec2(w,h));
  }

  void
  set(const PyramidLevel &below_level, const ivec2 &base_level_size, int level)
  {
    /*
      we need to pad the size by 1
      if the dimension is not a 
      multiple of 2^level:
     */
    ivec2 psize(base_level_size.x()>>level,
                base_level_size.y()>>level);

    if( (psize.x()<<level) != base_level_size.x()) {
      ++psize.x();
    }
    if( (psize.y()<<level) != base_level_size.y()) {
      ++psize.y();
    }                  
    init(psize);
    
    for(int y=0; y<height(); ++y) {
      for(int x=0; x<width(); ++x) {
        value(x, y, 
              below_level.value(2*x + 0, 2*y + 0)
              || below_level.value(2*x + 1, 2*y + 0)
              || below_level.value(2*x + 0, 2*y + 1)
              || below_level.value(2*x + 1, 2*y + 1));
      }
    }
  }
  
  bool
  value(int x, int y) const
  {
    if(in_range(x,y)) {
      std::pair<int, int> R(location(x, y));
      return (m_values[R.first]&R.second)!=0;
    }
    else {
      return false;
    }
  }
  
  void
  value(int x, int y, bool v)
  {
    if(in_range(x,y)) {
      std::pair<int, int> R(location(x,y));
      if(v) {
        m_values[R.first]|=R.second;
      } else {
        m_values[R.first]&=~R.second;
      }
    }
  }
      
  
  int
  width(void) const { return m_size.x(); }
  
  int
  height(void) const { return m_size.y(); }
  
  const ivec2&
  size(void) const { return m_size; }

  void
  swap(PyramidLevel &obj)
  {
    std::swap(m_size, obj.m_size);
    std::swap(m_values, obj.m_values);
  }

private:
  bool
  in_range(int x, int y) const
  {
    return x<width() && x>=0 && y>=0 && y<height();
  }

  static
  int
  array_size(const ivec2 &psize)
  {
    int entries;

    entries=psize.x()*psize.y();
    if(entries&31){
      return 1 + entries/32;
    } else {
      return entries/32;
    }
  }

  std::pair<int, int>
  location(int x, int y) const
  {
    //.first is index into m_values
    //.second is bit-mask 
    std::pair<int, int> R;
    int raw_idx, which_bit;
    
    raw_idx= x + y*width();
    which_bit=raw_idx&31; //same as %32
    R.first=raw_idx/32; //32 values per uint32_t
    R.second= (1<<which_bit);

    return R;
  }
  
  ivec2 m_size;
  std::vector<uint32_t> m_values;
};

class QueryWorkRoom
{
public:
  std::vector<WebCore::IntRect> rect;
};
  
class InterestingAlphaPyramid
{
public:

  void
  set(PyramidLevel &img)
  {
    int N(0);
    int w(img.width()), h(img.height());

    for(;w!=0 && h!=0; w/=2, h/=2, ++N)
      {}

    m_levels.resize(N);
    for(int level=0; level<N; ++level) {
      if(level==0) {
        m_levels[level].swap(img);
      } else {
        m_levels[level].set(m_levels[level-1], m_levels[0].size(), level);
      }
    }
  }

  bool
  query(const WebCore::IntRect &rect, QueryWorkRoom &work_room) const
  {
    if(m_levels.empty())
      return false;

    init_work_room(work_room, rect);
    return !work_room.rect.empty() 
      && query_implement(work_room.rect.size()-1, work_room.rect.back(), work_room);
  }

  bool
  query(const WebCore::IntRect &rect) const
  {
    QueryWorkRoom work_room;
    return query(rect, work_room);
  }

  bool
  query_brute(const WebCore::IntRect &rect) const
  {
    for(int y=rect.y(); y<rect.maxY() && y<level0().height(); ++y) {
      for(int x=rect.x(); x<rect.maxX() && x<level0().width(); ++x) {

	if(level0().value(x, y)) {
	  return true;
	}
      }
    }

    return false;
  }

  const PyramidLevel&
  level0(void) const { return m_levels[0]; }

private:
  void
  init_work_room(QueryWorkRoom &work_room,  const WebCore::IntRect &rect) const
  {
    /*
      we do not need all levels, just 
      those for which rect/2^N is non-zero..
     */
    work_room.rect.clear();
    work_room.rect.reserve(m_levels.size());

    WebCore::IntRect rect0(0, 0, level0().width(), level0().height());
    work_room.rect.push_back(WebCore::intersection(rect, rect0)); 
          
    for(uint32_t
          w=work_room.rect.front().width(), 
          h=work_room.rect.front().height(), 
          lod=1, 
          mask=1; 
        w!=0 and h!=0 and lod<m_levels.size(); 
        w/=2, h/=2, ++lod, mask=1|(mask<<1) ) {

      int x, y, maxX, maxY;

      x=work_room.rect.front().x()>>lod;
      maxX=work_room.rect.front().maxX()>>lod;

      y=work_room.rect.front().y()>>lod;
      maxY=work_room.rect.front().maxY()>>lod;

      if(work_room.rect.front().maxX()&mask) {
        ++maxX;
      }

      if(work_room.rect.front().maxY()&mask) {
        ++maxY;
      }

      if(x>=maxX or y>=maxY) {
        return;
      }

      work_room.rect.push_back(WebCore::IntRect(x, y, maxX-x, maxY-y));
    }
  }

  bool
  query_implement_level0(const WebCore::IntRect &rect_lod, const QueryWorkRoom &work_room) const
  {
    WebCore::IntRect rect( WebCore::intersection(rect_lod, work_room.rect[0]) );
    for(int y=rect.y(), end_y=rect.maxY(); y<end_y; ++y) {
      for(int x=rect.x(), end_x=rect.maxX(); x<end_x; ++x) {
        if(level0().value(x, y)) {
            return true;
         }
      }
    }

    return false;
  }

  bool
  query_implement(int level, 
                  const WebCore::IntRect &rect_lod,
                  const QueryWorkRoom &work_room) const
  {
    if(level==0) {
      return query_implement_level0(rect_lod, work_room);
    }

    /*
      For now we do a dead simple recursive
      query:
       - we check each of those booleans in rect_lod _intersect_ work_room.rect[level]
       - for those that are true we recurse, when we recurse we just double coordinate
         location, the intersection with work_room.rect[] makes sure we do not leak.
     */
    WebCore::IntRect rect( WebCore::intersection(rect_lod, work_room.rect[level]) );


    for(int y=rect.y(), end_y=rect.maxY(); y<end_y; ++y) {
      for(int x=rect.x(), end_x=rect.maxX(); x<end_x; ++x) {
        if(m_levels[level].value(x, y)) {
          
          WebCore::IntRect sub_rect(2*x, 2*y, 2, 2);
          if(query_implement(level-1, sub_rect, work_room)) {
            return true;
          }
        } // if(m_levels[level].value(x, y))
        
      } //for(x=..
    } //for(y=...

    return false;
  }

  std::vector<PyramidLevel> m_levels;
};


std::string
image_name(const WebCore::NativeImageWRATH & owner, const ivec2 & sz)
{
  std::ostringstream ostr;

  ostr << "WrappedWRATHImage" << sz
       << " --" << &owner;

  return ostr.str();
}


WRATHImage::BoundarySize compute_boundary_size(ivec2 image_size,
                                               ivec2 max_image_size_allowed)
{
  WRATHImage::BoundarySize R;

  if(image_size.x()<=0 or image_size.y()<=0) {
    return R;
  }

 
  if(image_size.x()+2 <= max_image_size_allowed.x()) {
    R.m_minX=1;
    R.m_maxX=1;
  } else if (image_size.x()+1 == max_image_size_allowed.x()) {
    R.m_minX=1;
  }

  if(image_size.y()+2 <= max_image_size_allowed.y()) {
    R.m_minY=1;
    R.m_maxY=1;
  } else if (image_size.y()+1 == max_image_size_allowed.y()) {
    R.m_minY=1;
  }

  return R;
}


int
wrap_around_clamp(int x, int pmin, int pmax)
{
  if(x<pmin)
    return pmax;

  if(x>pmax)
    return pmin;

  return x;
}


class WrappedWRATHImage: public WRATHImage
{
public:

    WrappedWRATHImage(const WebCore::NativeImageWRATH & owner,
                      const ivec2 & sz, 
                      const WRATHImage::ImageFormat & fmt,
                      const WRATHImage::BoundarySize &bd)
      : WRATHImage(image_name(owner, sz), sz, fmt, bd)
      , m_hasNonTrivialAlphaValues(false)
      , m_scaled(false)
      , m_scale_factors(1.0f, 1.0f)
      , m_origSize(0, 0)
      , m_owner(owner)
    {
    }

    virtual
    ~WrappedWRATHImage()
    {
      m_owner.onWrathImageGone();
    }

    bool m_hasNonTrivialAlphaValues, m_scaled;
    vec2 m_scale_factors;
    InterestingAlphaPyramid m_query_image;
    WebCore::IntSize m_origSize;
  
private:
    const WebCore::NativeImageWRATH & m_owner;

};


/*
  extraction from a QImage to a GL texture
  is based upon a template argument that handles
  the conversion. 
 */
template<typename T>
bool createPixelData(const QImage &qimg,
                     std::vector<uint8_t> &inout_data,
                     WRATHImage::ImageFormat &fmt,
                     const WRATHImage::BoundarySize &bd,
                     int &packing,
                     PyramidLevel &level0)
{
  ivec2 sz(qimg.width(), qimg.height());
  bool return_value(false);
  ivec2 sz_with_boundary(sz.x()+bd.m_minX+bd.m_maxX,
                         sz.y()+bd.m_minY+bd.m_maxY);
  vec2 start(-bd.m_minX, -bd.m_minY);
  ivec2 end(sz.x()+bd.m_maxX, sz.y()+bd.m_maxY);
  
  if(T::has_alpha_channel()) {
    level0.init(qimg.width(), qimg.height());
  }

  inout_data.resize(sizeof(typename T::pixel_type)*sz_with_boundary.x()*sz_with_boundary.y());
  c_array<uint8_t> data(inout_data);
  c_array<typename T::pixel_type> pixel_data;

  pixel_data=data.reinterpret_pointer<typename T::pixel_type>();

  for (int pixel_index=0, yy = start.y(); yy < end.y(); ++yy) {
    for (int xx = start.x(); xx < end.x(); ++xx, ++pixel_index) {
      int pt_x(wrap_around_clamp(xx, 0, sz.x()-1));
      int pt_y(wrap_around_clamp(yy, 0, sz.y()-1));
      QRgb pixel = qimg.pixel(pt_x, pt_y);
      uint8_t r = qRed(pixel);
      uint8_t g = qGreen(pixel);
      uint8_t b = qBlue(pixel);
      uint8_t a = qAlpha(pixel);
      bool interesting_alpha;
      
      pixel_data[pixel_index]=T::extract_pixel_value(r, g, b, a);
      
      interesting_alpha=T::alpha_is_interesting(a);
      level0.value(pt_x, pt_y, interesting_alpha);

      return_value=return_value or interesting_alpha;
    }    
  }

  T::set_image_format(fmt);
  switch(sizeof(typename T::pixel_type))
    {
    default:
      packing=1;
      break;

    case 2:
      packing=2;
      break;

    case 4:
      packing=4;
      break;
    }

  return T::has_alpha_channel() && return_value;
}


class RGBA8_fmt
{
public:
  typedef vecN<uint8_t, 4> pixel_type;
  
  static
  bool
  has_alpha_channel(void) { return true; }

  static
  void
  set_image_format(WRATHImage::ImageFormat &fmt)
  {
    fmt
      .pixel_data_format(GL_RGBA)
      .pixel_type(GL_UNSIGNED_BYTE)
      .internal_format(GL_RGBA);
  }

  static
  bool
  alpha_is_interesting(uint32_t a)
  {
    return (a!=0 and a!=255);
  }

  static
  pixel_type
  extract_pixel_value(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
  {
    return pixel_type(r, g, b, a);
  }
};

template<unsigned int red_bits, 
         unsigned int green_bits, 
         unsigned int blue_bits, 
         unsigned int alpha_bits,
         GLenum channel_format,
         GLenum pixel_format_type>
class Generic16bit_fmt
{
public:
  typedef uint16_t pixel_type;

  enum channel
    {
      red_channel=0,
      green_channel=1,
      blue_channel=2,
      alpha_channel=3,
    };

  static
  uint16_t
  extract_bits(uint16_t v, enum channel bt)
  {
    static uint32_t shifts[]=
      {
        green_bits+blue_bits+alpha_bits, //red
        blue_bits+alpha_bits, //green
        alpha_bits, //blue
        0, //alpha
      };

    static uint32_t extracts[]=
      {
        8-red_bits, 
        8-green_bits, 
        8-blue_bits, 
        8-alpha_bits
      };
    return ( v>>extracts[bt] ) << shifts[bt];
  }

  
  static
  bool
  has_alpha_channel(void) { return alpha_bits!=0; }

  static
  void
  set_image_format(WRATHImage::ImageFormat &fmt)
  {
    fmt
      .pixel_data_format(channel_format)
      .pixel_type(pixel_format_type)
      .internal_format(channel_format);
  }

  static
  bool
  alpha_is_interesting(uint32_t a)
  {
    uint32_t shift(8-alpha_bits);
    uint32_t all_up(0xFF);
    return has_alpha_channel()
      and ((a >> shift) != (all_up >> shift))
      and ( (a>>shift)!=0) ;
  }


  static
  pixel_type
  extract_pixel_value(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
  {
    return extract_bits(r, red_channel) |
      extract_bits(g, green_channel) |
      extract_bits(b, blue_channel) |
      extract_bits(a, alpha_channel);
  }
};

typedef Generic16bit_fmt<5, 6, 5, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5> RGBA565_fmt;
//typedef Generic16bit_fmt<4, 4, 4, 4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4> RGBA4444_fmt;
//typedef Generic16bit_fmt<5, 5, 5, 1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1> RGBA5551_fmt;




WrappedWRATHImage* createWRATHImage(const QImage & in_img, const WebCore::NativeImageWRATH & owner)
{
  WrappedWRATHImage *r;

  ivec2 sizeCap(WRATHImage::texture_atlas_dimension());
  bool scaled(false);
  vec2 scale_factor(1.0f, 1.0f);
  PyramidLevel level0;

  QImage img(in_img);
  if (in_img.width() > sizeCap.x() || in_img.height() > sizeCap.y()) {
      img = in_img.scaled(sizeCap.x(), sizeCap.y(), Qt::KeepAspectRatio);

      scaled=true;
      scale_factor.x()=static_cast<float>(img.width())/static_cast<float>(in_img.width());
      scale_factor.y()=static_cast<float>(img.height())/static_cast<float>(in_img.height());
  }

  ivec2 sz(img.width(), img.height());
  WRATHImage::ImageFormat fmt;
  WRATHImage::BoundarySize bd;

  bd=compute_boundary_size(sz, sizeCap);

  fmt
    .magnification_filter(GL_NEAREST)
    .minification_filter(GL_NEAREST);

  //r=WRATHNew WrappedWRATHImage(owner, sz, fmt);
  //WRATHQT::respecify_sub_image(r, img);
  //return r;

  
  QImage::Format format = img.format();
  std::vector<uint8_t> data;
  int packing(1);
  bool hasInterestingAlpha;

  /*
    TODO: a proper switch statement for various formats.
    For now we don't care and just use the presence
    of alpha to determine everything
   */
  if(img.hasAlphaChannel())
    {
      hasInterestingAlpha=createPixelData<RGBA8_fmt>(img, data, fmt, bd, packing, level0);
    }
  else
    {
      #ifdef HARMATTAN
      {
        hasInterestingAlpha=false;
        createPixelData<RGBA8_fmt>(img, data, fmt, bd, packing, level0);
      }
      #else
      {
        hasInterestingAlpha=createPixelData<RGBA565_fmt>(img, data, fmt, bd, packing, level0);
      }
      #endif
    }

  r=WRATHNew WrappedWRATHImage(owner, sz, fmt, bd);
  r->respecify_sub_image(0, //LOD
                         fmt.m_pixel_format, //pixel format
                         data, // pixel data
                         ivec2(-bd.m_minX, -bd.m_minY), //bottom left corner
                         sz+ivec2(bd.m_minX+bd.m_maxX, bd.m_minY+bd.m_maxY), //size
                         packing); //scanline alignment
  r->m_hasNonTrivialAlphaValues=img.hasAlphaChannel() && hasInterestingAlpha;
  r->m_scaled=scaled;
  r->m_scale_factors=scale_factor;
  r->m_origSize=WebCore::IntSize(in_img.width(), in_img.height());
  if(r->m_hasNonTrivialAlphaValues) {
    /*
      ICK, gross, we get to save the image data for the interesting
      alpha check.. sighs and groans.
     */
    r->m_query_image.set(level0);
  }

  return r;

}



}

namespace WebCore {


NativeImageWRATH::NativeImageWRATH(const QImage & img) :
    m_pixmap(),
    m_wrathImage(0)
{
    m_pixmap = adoptPtr(new QPixmap(QPixmap::fromImage(img)));
}

NativeImageWRATH::NativeImageWRATH(PassOwnPtr<QPixmap> pm) :
    m_pixmap(pm),
    m_wrathImage(0)
{
}

NativeImageWRATH::NativeImageWRATH(const NativeImageWRATH & other) :
    m_pixmap(),
    m_wrathImage(0)
{
    QPixmap* otherPixmap = other.getPixmap();
    m_pixmap = adoptPtr(new QPixmap(*otherPixmap));
}

NativeImageWRATH::~NativeImageWRATH()
{
    if(m_wrathImage)
        WRATHPhasedDelete(m_wrathImage);
}

QPixmap* NativeImageWRATH::getPixmap() const
{
    return m_pixmap.get();
}


bool NativeImageWRATH::hasAlpha(void) const
{
  WRATHImage *p;

  p=getWrathImage();
  return p->image_format(0).m_pixel_format.m_pixel_data_format==GL_RGBA
    or p->image_format(0).m_pixel_format.m_pixel_data_format==GL_ALPHA
    or p->image_format(0).m_pixel_format.m_pixel_data_format==GL_LUMINANCE_ALPHA;
}

bool NativeImageWRATH::hasNonTrivialAlpha(void) const
{
  WRATHImage *p;
  WrappedWRATHImage *q;

  p=getWrathImage();
  WRATHassert(dynamic_cast<WrappedWRATHImage*>(p));
  q=static_cast<WrappedWRATHImage*>(p);

  return q->m_hasNonTrivialAlphaValues && hasAlpha();

}

WRATHImage* NativeImageWRATH::getWrathImage() const
{
    if (!m_wrathImage)
    {
        QImage image = getPixmap()->toImage();
        m_wrathImage = createWRATHImage(image, *this);
    }

    return m_wrathImage;
}

void NativeImageWRATH::onWrathImageGone() const
{
    m_wrathImage = 0;
}

IntSize NativeImageWRATH::origSize() const
{
  const WRATHImage *im(getWrathImage());
  const WrappedWRATHImage *wim;
  
  WRATHassert(dynamic_cast<const WrappedWRATHImage*>(im)==im);
  wim=static_cast<const WrappedWRATHImage*>(im);
  
  return wim->m_origSize;
}

bool NativeImageWRATH::NonTrivialAlphaQuery::query(const FloatRect &src_rect, NativeImageWRATH *pim)
{
  WRATHassert(pim);
  const WRATHImage *im(pim->getWrathImage());
  const WrappedWRATHImage *wim;
  IntRect loc;
  
  WRATHassert(dynamic_cast<const WrappedWRATHImage*>(im)==im);
  wim=static_cast<const WrappedWRATHImage*>(im);

  if(!wim->m_hasNonTrivialAlphaValues) {
    m_wrath_image=im;
    m_result=false;
    return result();
  }
  
  if(wim->m_scaled) {
    FloatRect adjusted_src_rect;
    adjusted_src_rect=src_rect;
    adjusted_src_rect.scale(wim->m_scale_factors.x(), wim->m_scale_factors.y());

    loc=IntRect(adjusted_src_rect);
  } else {
    loc=IntRect(src_rect);
  }

  loc.intersect(IntRect(0, 0, 
                        wim->m_query_image.level0().width(), 
                        wim->m_query_image.level0().height()));

  if(im==m_wrath_image and loc==m_query_rect) {
    /*
      same query, nothing to do.
    */
    return result();
  } 


  m_query_rect=loc;
  m_wrath_image=im;
  m_result=wim->m_query_image.query(m_query_rect);

  
  return result();

}

}

#endif /* USE(WRATH) */
