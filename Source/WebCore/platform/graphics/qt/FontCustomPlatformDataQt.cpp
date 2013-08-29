/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/
#include "config.h"
#include "FontCustomPlatformData.h"

#include "FontPlatformData.h"
#include "SharedBuffer.h"
#include <QFontDatabase>
#include <QStringList>

namespace WebCore {

FontCustomPlatformData::~FontCustomPlatformData()
{
    QFontDatabase::removeApplicationFont(m_handle);
#if USE(WRATH)
    WRATHFontDatabase::release_unregistered_fonts(m_all_custom_fonts.begin(), m_all_custom_fonts.end());
#endif
}

FontPlatformData FontCustomPlatformData::fontPlatformData(int size, bool bold, bool italic, FontOrientation, TextOrientation, FontWidthVariant, FontRenderingMode)
{
    QFont font;
    font.setFamily(QFontDatabase::applicationFontFamilies(m_handle)[0]);
    font.setPixelSize(size);
    if (bold)
        font.setWeight(QFont::Bold);
    font.setItalic(italic);

#if USE(WRATH)
    /*
      check m_custom_fonts for italic and/or bold.
      An std::map seems like overkill, maybe change
      m_custom_fonts to an array and just check
      each element in the array...
     */
    std::map<vecN<bool,2>, WRATHFontDatabase::Font::const_handle>::const_iterator iter;

    iter=m_custom_fonts.find( vecN<bool, 2>(bold, italic) );
    if(iter!=m_custom_fonts.end())
      {
	return FontPlatformData(font, iter->second);
      }
    else if(!m_all_custom_fonts.empty())
      {
	return FontPlatformData(font, m_all_custom_fonts[0]);
      }
#endif

    return FontPlatformData(font);

}

FontCustomPlatformData* createFontCustomPlatformData(SharedBuffer* buffer)
{
    ASSERT_ARG(buffer, buffer);

    int id = QFontDatabase::addApplicationFontFromData(QByteArray(buffer->data(), buffer->size()));


    if (id == -1)
        return 0;

    Q_ASSERT(QFontDatabase::applicationFontFamilies(id).size() > 0);

    FontCustomPlatformData *data = new FontCustomPlatformData;
    data->m_handle = id;

#if USE(WRATH)
    std::vector<uint8_t> raw_bytes;

    raw_bytes.resize(buffer->size());
    std::copy(buffer->data(), buffer->data()+buffer->size(), raw_bytes.begin());
    
    WRATHFontDatabase::FontMemorySource::const_handle hs;
    hs=WRATHNew WRATHFontDatabase::FontMemorySource(raw_bytes);

    data->m_all_custom_fonts=WRATHFontDatabase::create_unregistered_fonts("CustomWebKitFont", hs);

    for(int idx=0; idx<data->m_all_custom_fonts.size(); ++idx)
      {
	vecN<bool, 2> b(data->m_all_custom_fonts[idx]->properties().m_bold, 
			data->m_all_custom_fonts[idx]->properties().m_italic);

	data->m_custom_fonts[b]=data->m_all_custom_fonts[idx];
      }
#endif


    return data;
}

bool FontCustomPlatformData::supportsFormat(const String& format)
{
    return equalIgnoringCase(format, "truetype") || equalIgnoringCase(format, "opentype");
}

}
