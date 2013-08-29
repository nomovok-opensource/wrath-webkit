#ifndef _DrawTextOfWRATH_
#define _DrawTextOfWRATH_

#include "PaintInfoOfWRATH.h"
#include "WRATHTextureFont.hpp"

namespace WebCore {

  /*
    corresponds to GraphicsContext::drawText command... with a
    variety of bits ignored...
   */
  class DrawnTextOfWRATHData:boost::noncopyable
  {
  public:
    explicit
    DrawnTextOfWRATHData(void);

    void
    text_visible(bool v)
    {
      if(m_text_node.widget()) {
        m_text_node.widget()->visible(v);
      }
      m_text_visible=v;
    }

    void
    update(ContextOfWRATH *ctx,
           const Font &fnt,
           const UChar *string_ptr, int string_len,
           const Color &fill_color,
           bool is_right_to_left,
           int from, int to);

    void
    update(ContextOfWRATH *ctx,
           const Font &fnt,
           const TextRun &run,
           const Color &fill_color,
           int from=0, int to=-1);

    void
    stroke_update(ContextOfWRATH *ctx,
                  const Font &fnt,
                  const UChar *string_ptr, int string_len,
                  const Color &fill_color,
                  float stroke_thickness,
                  const Color &stroke_color,
                  bool is_right_to_left, 
                  int from, int to);

    void
    stroke_update(ContextOfWRATH *ctx,
                  const Font &fnt,
                  const TextRun &run,
                  const Color &fill_color,
                  float stroke_thickness,
                  const Color &stroke_color,
                  int from=0, int to=-1);

    ContextOfWRATH::Node*
    node(void)
    {
      return m_text_node.widget();      
    }

    static
    FloatRect
    selectionRectForText(const Font &fnt, const TextRun& run, const FloatPoint& point, int h, int from=0, int to=-1);

    
    static
    range_type<float>
    width(const Font &fnt, const TextRun& run, int from, int to);

    static
    float
    width(const Font &fnt, const TextRun& run);

    static
    int
    offsetForPosition(const Font &fnt, const TextRun& run, float x, bool includePartialGlyphs);
    

  private:
    WRATHTextureFont *m_font;
    int m_pixel_size;
    Color m_fill_color;

    bool m_stroke_text, m_is_right_to_left;
    Color m_stroke_color;

    short m_word_spacing;
    short m_letter_spacing;

    //in units 16.16
    unsigned int m_stroke_thickness;

    //string drawn, as a WebCore::String (file JaveScriptCore/wtf/text/WTFString.h  
    //which is a utf16 string.
    std::vector<UChar> m_current_string;

    //the WRATH representation of that string
    WRATHTextData m_text;

    //m_text processed, i.e. formatted, etc.
    WRATHFormattedTextStream m_formatted_text;
    WRATHStateStream m_state_stream;

    range_type<int> m_display_range;

    //the node for the text, the node handles
    //the scaling, positioning and visibility of the text
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_text_node;

    //the actual DrawnText item:
    WebCore::ContextOfWRATH::PlainFamily::DrawnText::AutoDelete m_text_item;

    bool m_text_visible;
  };


  class DrawnTextOfWRATH:
    public PaintedWidgetsOfWRATHBase,
    public DrawnTextOfWRATHData
  {
  public:
    virtual
    void
    visible(bool v)
    {
      DrawnTextOfWRATHData::text_visible(v);
    }
  };
    

}

#endif
