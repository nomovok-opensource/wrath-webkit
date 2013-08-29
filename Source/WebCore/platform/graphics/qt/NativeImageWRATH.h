#ifndef NATIVEIMAGEWRATH_H
#define NATIVEIMAGEWRATH_H 

#include <config.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>
#include <QPixmap>

#include "FloatRect.h"
#include "IntRect.h"
#include "IntSize.h"

#if USE(WRATH)
#include "WRATHConfig.hpp"
#include "WRATHImage.hpp"
#endif

namespace WebCore {

#if USE(WRATH)

class NativeImageWRATH 
{
public:
    NativeImageWRATH(const QImage & img);
    NativeImageWRATH(PassOwnPtr<QPixmap> pm);
    NativeImageWRATH(const NativeImageWRATH & other);
    ~NativeImageWRATH();

    QPixmap* getPixmap() const;
    WRATHImage* getWrathImage() const;

    void onWrathImageGone() const;

  //returns true if and only if WRATHImage has an alpha channel
    bool hasAlpha(void) const;

  //returns true if and only if WRATHImage has an alpha channel
  //whose alpha values are not only full transparent and fully opaque.
    bool hasNonTrivialAlpha(void) const;

  //returns the original size of the image. If that size is
  //to big, then the image gets downscaled, thus is the case
  //that the original size and the size of the WRATHImage
  //returned by getWrathImage may or may not be the same
    IntSize origSize(void) const;

    class NonTrivialAlphaQuery
    {
    public:
      NonTrivialAlphaQuery(void):
        m_query_rect(0, 0, 0, 0),
        m_result(false),
        m_wrath_image(0)
      {}

      //return true if the underlying image data has a non-trivial alpha channel
      bool
      result(void) const
      {
        return m_wrath_image && m_result;
      }

      bool
      query(const FloatRect &src_rect, NativeImageWRATH*);

    private:
      IntRect m_query_rect;
      bool m_result;
      const WRATHImage *m_wrath_image;
    };
  

  //deleted the widget
    void clear(void);

private:
    typedef WRATHImage* WRATHImagePtr;

    NativeImageWRATH() {}
    OwnPtr<QPixmap> m_pixmap;
    mutable WRATHImagePtr m_wrathImage;

};

#endif /* USE(WRATH) */

} /* namespace WebCore */

#endif /* NATIVEIMAGEWRATH_H */
