
#include "config.h"


#include "InlineTextBox.h"
#include "Chrome.h"
#include "ChromeClient.h"
#include "Document.h"
#include "DocumentMarkerController.h"
#include "Editor.h"
#include "EllipsisBox.h"
#include "Frame.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "Page.h"
#include "PaintInfo.h"
#include "RenderArena.h"
#include "RenderBR.h"
#include "RenderBlock.h"
#include "RenderCombineText.h"
#include "RenderRubyRun.h"
#include "RenderRubyText.h"
#include "RenderTheme.h"
#include "Text.h"
#include "break_lines.h"
#include <wtf/AlwaysInline.h>
#include <boost/iterator_adaptors.hpp>

#include "DrawnTextOfWRATH.h"
#include "WRATHUTF16.hpp"
#include "WRATHTextDataStreamManipulator.hpp"


namespace
{
  bool is_start_of_2_character_code(UChar w)
  {
    uint16_t w1(static_cast<uint16_t>(w));

    if(w1<0xD800 or w1>0xDFFF)
      {
	return false;
      }

    if(w1>=0xDBFF or w1<=0xD800)
      {
	//bad utf16 stream.
	return false;
      }

    return true;
  }

  /*
    WebKit provides the display range relative to the
    start of the UTF16 string, not to actual full characters.
    If there are any characters in the string that require 
    2-uint16_t's to hold, then the character range is different.
   */
  range_type<int> compute_range(range_type<int> in_range_u16,
				const_c_array<UChar> data)
  {
    unsigned int I, lastI, count;
    range_type<int> R(0, 0);
    /*
      UTF16 is thankfully simple. A character either
      takes 1 or 2 uint16_t's.

      we want R.m_begin to be the number
      of full characters before in_range_u16.m_begin

      and we want R.m_end to be the number
      of full characters before in_range_u16.m_end
     */
    for(I=0, lastI=data.size(), count=0; I<lastI; ++I, ++count)
      {
	if(I==in_range_u16.m_begin)
	  {
	    R.m_begin=count;
	  }
	if(I==in_range_u16.m_end)
	  {
	    R.m_end=count;
	  }

	if(is_start_of_2_character_code(data[I]))
	  {
	    ++I;
	  }
      }

    if(I==in_range_u16.m_end)
      {
	R.m_end=count;
      }

    return R;
  }
  
  /*
    formatting notes if it was a WRATHColumnFormatter:

    WRATHColumnFormatter::LayoutSpecification()
    .eat_white_spaces(false)
    .ignore_control_characters(false)
    .add_leading_eol(false)
    .word_space_on_line_begin(true)
    .horizontal_pen_advance(is_right_to_left?
                            WRATHFormatter::decrease_coordinate:
                            WRATHFormatter::increase_coordinate));
    
  */
  class PrivateForamtter:public WRATHFormatter
  {
  public:
    /*
      We know that the state stream does not change values and
      in fact it's values are to be:

      WRATHText::set_font(m_font)
      WRATHText::set_scale(1.0f)
      WRATHText::set_pixel_size(32.0f)
      WRATHText::set_color(fixed_color) //does not affect formatting
      WRATHText::set_letter_spacing_type(WRATHText::letter_spacing_absolute)
      WRATHText::set_letter_spacing(m_letter_spacing)
      WRATHText::set_word_spacing(m_word_spacing);

      Additionally since the text is pre-massaged we know that
      various control codes like \n and \t will not appear as well.
      
     */
    PrivateForamtter(bool is_right_to_left, const WebCore::Font *fnt):
      m_is_right_to_left(is_right_to_left),
      m_font(fnt)
    {}



    virtual
    enum screen_orientation_type
    screen_orientation(void)
    {
      return y_increases_downward;
    }

    template<typename iterator, typename F>
    static
    range_type<float>
    format_size_helper_implement(const WebCore::Font &fnt, bool is_right_to_left,
                                 const F &func,
                                 iterator begin, iterator end, 
                                 int from, int to,
                                 float in_x_position, int *out_offset,
                                 std::vector<WRATHFormatter::glyph_instance> *out_data,
                                 std::vector<std::pair<int, LineData> > *out_eols,
                                 pen_position_return_type *out_pen_positin);
    

    class extract_value_utf16
    {
    public:
      uint32_t
      operator()(const WRATHUTF16<const UChar*>::iterator &iter) const
      {
        return *iter;
      }
    };


    static
    range_type<float>
    format_size_helper(const WebCore::Font &fnt, const WebCore::TextRun& run, int from, int to,
                       float in_x_position, int *out_offset)
    {
      WRATHUTF16<const UChar*> UTF16(run.characters(), run.characters()+run.length());
      std::vector<WRATHFormatter::glyph_instance> *no_out_data(0);
      std::vector<std::pair<int, LineData> > *no_out_eols(0);
      pen_position_return_type *no_pen_position(0);

      return format_size_helper_implement(fnt, run.rtl(), 
                                          extract_value_utf16(), UTF16.begin(), UTF16.end(), 
                                          from, to, 
                                          in_x_position, out_offset, 
                                          no_out_data, no_out_eols, 
                                          no_pen_position);
    }


    class extract_value
    {
    public:
      uint32_t
      operator()(const std::vector<WRATHTextData::character>::const_iterator &ch) const
      {
        return ch->character_code().m_value;
      } 
    };

    
    virtual
    pen_position_return_type 
    format_text(const WRATHTextData &raw_data,
                const WRATHStateStream &,
                std::vector<WRATHFormatter::glyph_instance> &out_data,
                std::vector<std::pair<int, LineData> > &out_eols)
    {
      float in_x_positin(0.0f);
      int *no_out_offset(0);
      pen_position_return_type R;

      

      format_size_helper_implement(*m_font, m_is_right_to_left, 
                                   extract_value(),
                                   raw_data.character_data().begin(),
                                   raw_data.character_data().end(), 
                                   0, raw_data.character_data().size(), //from and to
                                   in_x_positin, no_out_offset,
                                   &out_data, &out_eols, &R);

      return R;
    }


  private:
    bool m_is_right_to_left;
    const WebCore::Font *m_font;
  };
}


template<typename iterator, typename F>
range_type<float>
PrivateForamtter::
format_size_helper_implement(const WebCore::Font &fnt, bool is_right_to_left,
                             const F &func,
                             iterator begin, iterator end,
                             int from, int to,
                             float in_x_position, int *out_offset,
                             std::vector<WRATHFormatter::glyph_instance> *out_data,
                             std::vector<std::pair<int, LineData> > *out_eols,
                             pen_position_return_type *out_pen_position)
{
  WRATHTextureFont *wrath_font;
  range_type<float> return_value(0.0f, 0.0f);
  int current_index(0);
  float pen_position(0.0f), last_pen_position(0.0f);
  float letter_spacing;
  float word_spacing;
  float sc;
  WRATHTextureFont *null_font(0);
  WRATHTextureFont::font_glyph_index prev_glyph(null_font, WRATHTextureFont::glyph_index_type());
  int fact;
  int run_length(std::distance(begin, end));
  LineData L(0, run_length);

  to = (to == -1 ? run_length : to);
  wrath_font=fnt.fontOfWRATH();
  fact=is_right_to_left?-1:1;

  /*
    first we do the computation in units of the WRATH font size...
   */
  sc=static_cast<float>(fnt.pixelSize())/static_cast<float>(fnt.fontOfWRATH()->pixel_size());
  letter_spacing=static_cast<float>(fact*fnt.letterSpacing())/sc;
  word_spacing=static_cast<float>(fact*fnt.wordSpacing())/sc;
  in_x_position/=sc;



  for(iterator iter=begin; iter!=end && current_index<to; ++iter, ++current_index)
    {
      uint32_t character_code;
      UChar as_uchar;
      WRATHFormatter::glyph_instance c;
      
      last_pen_position=pen_position;
      character_code=func(iter);
      as_uchar=static_cast<UChar>(character_code);
      
      if(current_index==from)
        {
          return_value.m_begin=pen_position;
        }
      
      
      /*
        first filter the character code.
      */
      if(WebCore::Font::treatAsSpace(as_uchar))
        {
          character_code=WTF::Unicode::space;
        }
      else if(WebCore::Font::treatAsZeroWidthSpace(as_uchar))
        {
          character_code=zeroWidthSpace;
        }
      
      c.m_position.x()=pen_position;
      c.m_position.y()=0.0f;  
      c.m_glyph=0;

      /*
        treat zero-width spaces as if they did not exist,
        i.e. no glyph, and no effect on spacing
        [should they induce a word or letter spacing though?]
      */
      if(character_code!=zeroWidthSpace)
        {
          WRATHTextureFont::font_glyph_index G(null_font, WRATHTextureFont::glyph_index_type()); 
          int kerni;
          float kern;
          const WRATHTextureFont::glyph_data_type *gl;
          bool is_space;

          G=wrath_font->glyph_index_meta(WRATHTextureFont::character_code_type(character_code));          
          is_space=(character_code==space);

          gl=(G.first && G.second.valid())?
            &G.first->glyph_data(G.second):
            0;
          
          if(is_right_to_left)
            {
              kerni=WRATHTextureFont::kerning_offset(G, prev_glyph).x();
            }
          else
            {
              kerni=WRATHTextureFont::kerning_offset(prev_glyph, G).x();
            }
          kern=static_cast<float>(kerni*fact)/64.0f;
          pen_position+=kern;

          
          c.m_position.x()+=kern;
	  if(gl && gl->texel_size()!=ivec2(0,0))
	    {
	      c.m_glyph=gl;
	    }

          if(is_space)
            {
              if(current_index!=0)
                {
                  pen_position+=word_spacing;
                }
              prev_glyph.first=0;
	      prev_glyph.second=WRATHTextureFont::glyph_index_type();
            }
          else
            {
              prev_glyph=G;
              pen_position+=letter_spacing;
            }

          if(gl)
            {
              vec2 orig(gl->origin());
              vec2 bb_size(gl->bounding_box_size());
              vec2 bb(orig+bb_size);
              
              pen_position+=gl->advance().x();
              L.m_max_ascend=std::max(L.m_max_ascend, bb.y());
              L.m_max_descend=std::max(L.m_max_descend, -bb.y());
            }

          if(out_offset && last_pen_position<=in_x_position && in_x_position<=pen_position)
            {
              *out_offset=current_index;
              out_offset=0;
            }
        }

      if(out_data)
        {
          out_data->push_back(c);
        }
    }  

  return_value.m_end=pen_position;

  /*
    now convert to units of the fnt.pixelSize():
   */
  return_value.m_begin*=sc;
  return_value.m_end*=sc;
 
  if(out_eols)
    {
      L.m_pen_position_end=vec2(pen_position, 0.0f);
      L.m_pen_position_start=vec2(0.0f, 0.0f);
      out_eols->push_back(std::pair<int, LineData>(0, L));
    }

  if(out_pen_position)
    {
      out_pen_position->m_exact_pen_position=vec2(pen_position, 0.0f);
      out_pen_position->m_descend_start_pen_position=vec2(0.0f, L.m_max_descend);
    }

  return return_value;
}







namespace WebCore
{
  

  void
  DrawnTextOfWRATHData::
  update(ContextOfWRATH *ctx,
         const Font &fnt,
         const UChar *string_ptr, int string_len,
         const Color &fill_color,
         bool is_right_to_left,
         int display_start, int display_end)
  {
    WebCore::ContextOfWRATH::AutoPushNode autoPush(ctx, m_text_node);


    bool format_dirty(false), text_dirty(false);
    range_type<int> display_range(display_start, display_end);

    if(string_len!=static_cast<int>(m_current_string.size())
       or !std::equal(m_current_string.begin(), m_current_string.end(), string_ptr))
      {
        WRATHUTF16<const UChar*> UTF16(string_ptr, string_ptr+string_len);

        text_dirty=true;
        m_current_string.resize(string_len);
        std::copy(string_ptr, string_ptr+string_len,
                  m_current_string.begin());

        m_text.clear();
        for(WRATHUTF16<const UChar*>::iterator iter=UTF16.begin(),
              end=UTF16.end(); iter!=end; ++iter)
          {
            uint32_t character_code;
            UChar as_uchar;

            character_code=*iter;
            as_uchar=static_cast<UChar>(character_code);

            if(Font::treatAsSpace(as_uchar))
              {
                character_code=space;
              }
            else if(Font::treatAsZeroWidthSpace(as_uchar))
              {
                /*
                  just ignore zero-width characters.
                 */
                character_code=zeroWidthSpace;
              }
            m_text.push_back(character_code);          
          }
      }

    
    /*
      recall that the String class is defined in JaveScriptCore/wtf/text/WTFString.h
    */
    
    if(m_font!=fnt.fontOfWRATH()
       or m_pixel_size!=fnt.pixelSize()
       or m_word_spacing!=fnt.wordSpacing()
       or m_letter_spacing!=fnt.letterSpacing()
       or m_is_right_to_left!=is_right_to_left
       or text_dirty)
      {
        PrivateForamtter::handle h;

        format_dirty=true;
        
        m_font=fnt.fontOfWRATH();
        m_word_spacing=fnt.wordSpacing();
        m_letter_spacing=fnt.letterSpacing();
        m_pixel_size=fnt.pixelSize();
        m_is_right_to_left=is_right_to_left;

        /*
          note that we se the text scaling factor as 1.0(!)
          this is because we let the node handle scaling for
          the text item.
         */     


        h=WRATHNew PrivateForamtter(m_is_right_to_left, &fnt);
        m_formatted_text.set_text(h, m_text, m_state_stream);
      }
    


    if(!m_text_item.widget() 
       or m_fill_color!=fill_color 
       or format_dirty
       or m_display_range.m_begin!=display_range.m_begin
       or m_display_range.m_end!=display_range.m_end)
      {
        vec4 color_as_float;
                
        m_stroke_text=false;
        m_fill_color=fill_color;
        m_display_range=display_range;

        m_fill_color.getRGBA(color_as_float.x(),
                             color_as_float.y(),
                             color_as_float.z(),
                             color_as_float.w());
        /*
          Note that WRATHStateStream::increment_time_to_value
          is _NEVER_ called on m_state_stream
         */
        WRATHassert(m_state_stream.time_value()==0);
        m_state_stream << WRATHText::set_font(m_font)
                       << WRATHText::set_pixel_size(m_font->pixel_size())
                       << WRATHText::set_scale(1.0f)
                       << WRATHText::set_color(color_as_float);
          
	range_type<int> range_in_characters(compute_range(m_display_range, 
							  const_c_array<UChar>(string_ptr, string_len)));


        if(!m_text_item.widget()) 
          {
             /*
               [WRATH-DANGER]: if we draw the text as transparent, we
               get the text to be AA, but there is a risk that if there
               are other transparent items, the z-ordering is fuzzed.
               We could also choose WRATHWidgetGenerator::text_opaque_non_aa
               and later rely on a post-process pass to do AA.
             */
            ctx->add_text(m_text_item,
                          WRATHWidgetGenerator::Text(range_in_characters,
                                                     m_formatted_text,
                                                     m_state_stream),
                          //WRATHWidgetGenerator::text_transparent,
                          WRATHWidgetGenerator::text_opaque,
                          //WRATHWidgetGenerator::text_opaque_non_aa,
                          WRATHWidgetGenerator::TextDrawerPacker(),
                          WRATHWidgetGenerator::TextDrawOrder(TextPassEnumerationOfWRATH));
          } 
        else 
          {
            m_text_item.widget()->properties()->clear();
            m_text_item.widget()->properties()->add_text(range_in_characters,
                                                         m_formatted_text,
                                                         m_state_stream);
            ctx->update_generic(m_text_item); 
          }

      }
    else
      {
        ctx->update_generic(m_text_item); 
      }
    
    float sc;
    sc=static_cast<float>(fnt.pixelSize())/static_cast<float>(fnt.fontOfWRATH()->pixel_size());

    m_text_node.widget()->scaling_factor(sc);
    m_text_node.widget()->visible(m_text_visible);
   
  }

  void
  DrawnTextOfWRATHData::
  stroke_update(ContextOfWRATH *ctx,
                const Font &fnt,
                const UChar *string_ptr, int string_len,
                const Color &fill_color,
                float stroke_thickness,
                const Color &stroke_color,
                bool is_right_to_left,
                int display_start, int display_end)
  {
    /*
      [WRATH-TODO]: use WRATH to stroke text...
     */
    update(ctx, fnt, string_ptr, string_len, fill_color, is_right_to_left, display_start, display_end);
  }

  void
  DrawnTextOfWRATHData::
  update(ContextOfWRATH *ctx,
         const Font &fnt,
         const TextRun &run,
         const Color &fill_color,
         int from, int to)
  {
    to = (to == -1 ? run.length() : to);
    update(ctx, fnt, 
           run.characters(), run.length(), fill_color, run.rtl(), from, to);
           
  }

  void
  DrawnTextOfWRATHData::
  stroke_update(ContextOfWRATH *ctx,
                const Font &fnt,
                const TextRun &run,
                const Color &fill_color,
                float stroke_thickness,
                const Color &stroke_color,
                int from, int to)
  {
    to = (to == -1 ? run.length() : to);
    stroke_update(ctx, fnt, 
                  run.characters(), run.length(), fill_color, 
                  stroke_thickness, stroke_color, 
                  run.rtl(), from, to);
           
  }


  DrawnTextOfWRATHData::
  DrawnTextOfWRATHData(void):
    m_font(0),
    m_pixel_size(-1),
    m_display_range(-1, -1),
    m_text_visible(true)
  {}

  

  range_type<float>
  DrawnTextOfWRATHData::
  width(const Font &fnt, const TextRun& run, int from, int to)
  {
    to = (to == -1 ? run.length() : to);
    return PrivateForamtter::format_size_helper(fnt, run, from, to, 0.0f, 0);
  }

  FloatRect
  DrawnTextOfWRATHData::
  selectionRectForText(const Font &fnt, const TextRun& run, const FloatPoint& point, int h, int from, int to)
  {
    range_type<float> R;

    R=width(fnt, run, from, to);
    if(R.m_begin>R.m_end)
      {
        std::swap(R.m_begin, R.m_end);
      }

    return FloatRect(point.x()+R.m_begin, point.y(),
                     R.m_end-R.m_begin, h);
  }

  float
  DrawnTextOfWRATHData::
  width(const Font &fnt, const TextRun& run)
  {
    range_type<float> R;

    R=width(fnt, run, 0, -1);
    return std::abs(R.m_end - R.m_begin) + run.expansion();
  }

  int
  DrawnTextOfWRATHData::
  offsetForPosition(const Font &fnt, const TextRun& run, float x, bool)
  {
    range_type<float> R;
    int return_value(0);

    R=PrivateForamtter::format_size_helper(fnt, run, 0, -1, x, &return_value);
    return return_value;
  }

}
