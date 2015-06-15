/*
 * This file is part of BlendInt (a Blender-like Interface Library in
 * OpenGL).
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is free
 * software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BlendInt.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#include <blendint/font/ft-face.hpp>

#include <blendint/stock/theme.hpp>
#include <blendint/stock/shaders.hpp>

#include <blendint/gui/text.hpp>

namespace BlendInt {

Text::Text (const String& text)
    : AbstractForm(),
      ascender_(0),
      descender_(0),
      vao_(0),
      text_(text)
{
  int width;
  std::vector<GLfloat> verts;
  GenerateTextVertices(verts, &width, &ascender_, &descender_);

  set_size(width, ascender_ - descender_);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  vbo_.generate();
  vbo_.bind();
  vbo_.set_data(sizeof(GLfloat) * verts.size(), &verts[0]);

  glEnableVertexAttribArray(AttributeCoord);
  glVertexAttribPointer(AttributeCoord, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
  vbo_.reset();
}

Text::Text (const Text& text)
    : AbstractForm(),
      ascender_(0),
      descender_(0),
      vao_(0),
      text_(text.text_)
{
  int width;
  std::vector<GLfloat> verts;
  GenerateTextVertices(verts, &width, &ascender_, &descender_);

  set_size(width, ascender_ - descender_);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  vbo_.generate();
  vbo_.bind();
  vbo_.set_data(sizeof(GLfloat) * verts.size(), &verts[0]);

  glEnableVertexAttribArray (AttributeCoord);
  glVertexAttribPointer (AttributeCoord, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
  vbo_.reset();
}

Text::~Text ()
{
  glDeleteVertexArrays(1, &vao_);
}

void Text::Add (const String& text)
{
  text_.append(text);

  ReloadBuffer();
}

void Text::Insert (size_t index, const String& text)
{
  if(text.empty() || (index > text_.length())) {
    text_.append(text);
  } else {
    text_.insert(index, text);
  }

  ReloadBuffer();
}

void Text::SetText (const String& text)
{
  text_ = text;
  ReloadBuffer();
}

void Text::Erase(size_t index, size_t count)
{
  text_.erase(index, count);

  ReloadBuffer();
}
    
void Text::SetFont(const Font& font)
{
  if(font_ == font) return;
  font_ = font;

  ReloadBuffer();
}

size_t Text::GetTextWidth (size_t length, size_t start, bool count_kerning) const
{
  size_t width = font_.GetTextWidth(text_, length, start);

  if(count_kerning && font_.has_kerning()) {

    int left_kerning = 0;
    int right_kerning = 0;

    if(start > 0) {
      left_kerning = font_.GetKerning(text_[start - 1], text_[start]).x;
    }

    size_t last = start + length;
    if(last < (text_.length() - 1)) {
      right_kerning = font_.GetKerning(text_[last], text_[last + 1]).x;
    }

    width = width + left_kerning + right_kerning;
  }

  return width;
}

Text& Text::operator = (const Text& orig)
{
  font_ = orig.font_;
  text_ = orig.text_;

  ReloadBuffer();

  return *this;
}

Text& Text::operator = (const String& text)
{
  text_ = text;
  ReloadBuffer();
  return *this;
}

void Text::PerformSizeUpdate (int width, int height)
{
  // Do nothing, this form does not allow resize
}

void Text::Draw (int x, int y, const float* color_ptr, short gamma,
                 float rotate, float scale_x, float scale_y) const
{
  shaders()->widget_text_program()->use();

  glActiveTexture(GL_TEXTURE0);

  font_.bind_texture();

  glUniform2f(shaders()->location(Shaders::WIDGET_TEXT_POSITION),
              pixel_size(x),
              pixel_size(y));
  glUniform4fv(shaders()->location(Shaders::WIDGET_TEXT_COLOR), 1, color_ptr);
  glUniform1i(shaders()->location(Shaders::WIDGET_TEXT_TEXTURE), 0);

  glBindVertexArray(vao_);
  size_t str_len = text_.length();
  for(size_t i = 0; i < str_len; i++) {
    glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
  }
}

void Text::DrawInRect (const Rect& rect,
                       int align,
                       const float* color_ptr,
                       short gamma,
                       float rotate,
                       bool scale) const
{
  if(rect.zero()) return;

  int x = rect.left();
  int y = rect.bottom();

  if(align & AlignLeft) {
    x = rect.left();
  } else if (align & AlignRight) {
    x = rect.right() - size().width();
  } else if (align & AlignHorizontalCenter) {
    x = rect.hcenter() - size().width() / 2;
  }

  if (align & AlignBaseline) {  // AlignBaseLine will override VerticalCenter

    y = rect.bottom() + (rect.height() - font_.height()) / 2 - font_.descender();

    // A workaround for Adobe Source Han Sans
    int diff = font_.ascender() - font_.descender();
    if(diff < font_.height()) {
      y += (font_.height() - diff - 1) / 2;
    }

  } else {

    if(align & AlignTop) {
      y = rect.top() - size().height();
    } else if (align & AlignBottom) {
      y = rect.bottom();
    } else if (align & AlignVerticalCenter) {
      y = (rect.y() + rect.height()) - size().height() / 2;
    }

  }

  shaders()->widget_text_program()->use();

  glActiveTexture(GL_TEXTURE0);

  font_.bind_texture();

  glUniform2f(shaders()->location(Shaders::WIDGET_TEXT_POSITION),
              pixel_size(x),
              pixel_size(y));
  glUniform4fv(shaders()->location(Shaders::WIDGET_TEXT_COLOR), 1, color_ptr);
  glUniform1i(shaders()->location(Shaders::WIDGET_TEXT_TEXTURE), 0);

  glBindVertexArray(vao_);

  const Glyph* g = 0;
  int max = 0;
  int count = 0;
  String::const_iterator next_it;
  Kerning kerning;

  for(String::const_iterator it = text_.begin(); it != text_.end(); it++)
  {
    g = font_.glyph(*it);

    if(font_.has_kerning()) {

      next_it = it + 1;
      if(next_it != text_.end()) {
        kerning = font_.GetKerning(*it, *next_it, Font::KerningDefault);
        max += static_cast<int>((g->advance_x + kerning.x) / theme()->pixel());
      } else {
        max += static_cast<int>(g->advance_x / theme()->pixel());
      }

    } else {
      max += static_cast<int>(g->advance_x / theme()->pixel());
    }

    if ((align & AlignJustify) && (max > rect.width())) break;

    glDrawArrays(GL_TRIANGLE_STRIP, count * 4, 4);

    count++;
  }
}

void Text::Draw (int x, int y, size_t length, size_t start,
                 short gamma) const
{
  Color color(0x000000FF);
  Draw(x, y, length, start, color, gamma);
}

void Text::Draw (int x, int y, size_t length, size_t start,
                 const Color& color, short gamma) const
{
  shaders()->widget_text_program()->use();

  glActiveTexture(GL_TEXTURE0);

  font_.bind_texture();

  glUniform2f(shaders()->location(Shaders::WIDGET_TEXT_POSITION),
              pixel_size(x),
              pixel_size(y));
  glUniform4fv(shaders()->location(Shaders::WIDGET_TEXT_COLOR), 1, color.data());
  glUniform1i(shaders()->location(Shaders::WIDGET_TEXT_TEXTURE), 0);

  glBindVertexArray(vao_);
  size_t str_len = text_.length();
  size_t last = std::min(start + length, str_len);
  for(size_t i = start; i < last; i++) {
    glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
  }
}

void Text::DrawWithin (int x, int y, int width, short gamma) const
{
  Color color(0x000000FF);
  DrawWithin(x, y, width, color, gamma);
}

void Text::DrawWithin (int x, int y, int width, const Color& color,
                       short gamma) const
{
  if(width <= 0) return;

  shaders()->widget_text_program()->use();

  glActiveTexture(GL_TEXTURE0);

  font_.bind_texture();

  glUniform2f(shaders()->location(Shaders::WIDGET_TEXT_POSITION),
              pixel_size(x),
              pixel_size(y));
  glUniform4fv(shaders()->location(Shaders::WIDGET_TEXT_COLOR), 1, color.data());
  glUniform1i(shaders()->location(Shaders::WIDGET_TEXT_TEXTURE), 0);

  glBindVertexArray(vao_);

  const Glyph* g = 0;
  int max = 0;
  int count = 0;
  String::const_iterator next_it;
  Kerning kerning;

  for(String::const_iterator it = text_.begin(); it != text_.end(); it++)
  {
    g = font_.glyph(*it);

    if(font_.has_kerning()) {

      next_it = it + 1;
      if(next_it != text_.end()) {
        kerning = font_.GetKerning(*it, *next_it, Font::KerningDefault);
        max += static_cast<int>((g->advance_x + kerning.x) / theme()->pixel());
      } else {
        max += static_cast<int>(g->advance_x / theme()->pixel());
      }

    } else {
      max += static_cast<int>(g->advance_x / theme()->pixel());
    }

    if (max > width) break;

    glDrawArrays(GL_TRIANGLE_STRIP, count * 4, 4);

    count++;
  }
}

int Text::DrawWithCursor(int x, int y, size_t index, size_t start, int width, const Color &color, short gamma) const
{
  int retval = 0;
        
  if(width <= 0) return retval;

  const Glyph* g = 0;
  int max = 0;
  size_t count = start;
  Kerning kerning;
  size_t i = 0;
  size_t n = 0;
  int ox = 0;

  for(; (i < start) && (i < text_.length()); i++) {
    g = font_.glyph(text_[i]);

    if(font_.has_kerning()) {

      n = i + 1;
      if(n != text_.length()) {
        kerning = font_.GetKerning(text_[i], text_[n], Font::KerningDefault);
        ox -= static_cast<int>((g->advance_x + kerning.x) / theme()->pixel());
      } else {
        ox -= static_cast<int>(g->advance_x / theme()->pixel());
      }

    } else {
      ox -= static_cast<int>(g->advance_x / theme()->pixel());
    }
  }
        
  shaders()->widget_text_program()->use();
        
  glActiveTexture(GL_TEXTURE0);
        
  font_.bind_texture();
        
  glUniform2f(shaders()->location(Shaders::WIDGET_TEXT_POSITION),
              pixel_size(x + ox),
              pixel_size(y));
  glUniform4fv(shaders()->location(Shaders::WIDGET_TEXT_COLOR), 1, color.data());
  glUniform1i(shaders()->location(Shaders::WIDGET_TEXT_TEXTURE), 0);
        
  glBindVertexArray(vao_);

  int tmp = 0;
  while(i < text_.length()) {

    g = font_.glyph(text_[i]);

    if(count <= index) retval = tmp;

    if(font_.has_kerning()) {

      n = i + 1;
      if(n != text_.length()) {
        kerning = font_.GetKerning(text_[i], text_[n], Font::KerningDefault);
        tmp += static_cast<int>((g->advance_x + kerning.x) / theme()->pixel());
      } else {
        tmp += static_cast<int>(g->advance_x / theme()->pixel());
      }

    } else {
      tmp += static_cast<int>(g->advance_x / theme()->pixel());
    }

    if (tmp > width) {
      break;
    } else {
      max = tmp;
    }

    glDrawArrays(GL_TRIANGLE_STRIP, count * 4, 4);

    i++;
    count++;
  }
        
  if(count <= index) retval = max;

  return retval;
}

int Text::DrawWithCursor(int x, int y, size_t index, size_t start, int width, short gamma) const
{
  Color color(0x000000FF);

  return DrawWithCursor(x, y, index, start, width, color, gamma);
}
    
void Text::GenerateTextVertices(std::vector<GLfloat> &verts, int* width_ptr, int* ascender_ptr, int* descender_ptr)
{
  size_t buf_size = text_.length() * 4 * 4;
  if(verts.size() != buf_size) verts.resize(buf_size, 0.f);
        
  int w = 0;	// width
  int a = 0;	// ascender
  int d = 0;	// descender
  const Glyph* g = 0;

  String::const_iterator next_it;

  int count = 0;
  if(font_.has_kerning()) {

    Kerning kerning;
    for(String::const_iterator it = text_.begin(); it != text_.end(); it++)
    {
      g = font_.glyph(*it);

      verts[count * 16 + 0] = w + g->bitmap_left;
      verts[count * 16 + 1] = g->bitmap_top - g->bitmap_height;
      verts[count * 16 + 2] = g->offset_u;
      verts[count * 16 + 3] = g->offset_v + g->bitmap_height;

      verts[count * 16 + 4] = w + g->bitmap_left + g->bitmap_width;
      verts[count * 16 + 5] = g->bitmap_top - g->bitmap_height;
      verts[count * 16 + 6] = g->offset_u + g->bitmap_width;
      verts[count * 16 + 7] = g->offset_v + g->bitmap_height;

      verts[count * 16 + 8] = w + g->bitmap_left;
      verts[count * 16 + 9] = g->bitmap_top;
      verts[count * 16 + 10] = g->offset_u;
      verts[count * 16 + 11] = g->offset_v;

      verts[count * 16 + 12] = w + g->bitmap_left + g->bitmap_width;
      verts[count * 16 + 13] = g->bitmap_top;
      verts[count * 16 + 14] = g->offset_u + g->bitmap_width;
      verts[count * 16 + 15] = g->offset_v;

      next_it = it + 1;
      if(next_it != text_.end()) {
        kerning = font_.GetKerning(*it, *next_it, Font::KerningDefault);
        w += (g->advance_x + kerning.x);
      } else {
        w += g->advance_x;
      }
      a = std::max(g->bitmap_top, a);
      d = std::min(g->bitmap_top - g->bitmap_height, d);

      count++;
    }

  } else {

    for(String::const_iterator it = text_.begin(); it != text_.end(); it++)
    {
      g = font_.glyph(*it);

      verts[count * 16 + 0] = w + g->bitmap_left;
      verts[count * 16 + 1] = g->bitmap_top - g->bitmap_height;
      verts[count * 16 + 2] = g->offset_u;
      verts[count * 16 + 3] = g->offset_v + g->bitmap_height;

      verts[count * 16 + 4] = w + g->bitmap_left + g->bitmap_width;
      verts[count * 16 + 5] = g->bitmap_top - g->bitmap_height;
      verts[count * 16 + 6] = g->offset_u + g->bitmap_width;
      verts[count * 16 + 7] = g->offset_v + g->bitmap_height;

      verts[count * 16 + 8] = w + g->bitmap_left;
      verts[count * 16 + 9] = g->bitmap_top;
      verts[count * 16 + 10] = g->offset_u;
      verts[count * 16 + 11] = g->offset_v;

      verts[count * 16 + 12] = w + g->bitmap_left + g->bitmap_width;
      verts[count * 16 + 13] = g->bitmap_top;
      verts[count * 16 + 14] = g->offset_u + g->bitmap_width;
      verts[count * 16 + 15] = g->offset_v;

      w += (g->advance_x);
      a = std::max(g->bitmap_top, a);
      d = std::min(g->bitmap_top - g->bitmap_height, d);

      count++;
    }

  }

  if(width_ptr) *width_ptr = static_cast<int>(w / theme()->pixel());
  if(ascender_ptr) *ascender_ptr = static_cast<int>(a / theme()->pixel());
  if(descender_ptr) *descender_ptr = static_cast<int>(d / theme()->pixel());
}
    
void Text::ReloadBuffer()
{
  int width;
  std::vector<GLfloat> verts;
  GenerateTextVertices(verts, &width, &ascender_, &descender_);
        
  vbo_.bind();
  vbo_.set_data(sizeof(GLfloat) * verts.size(), &verts[0]);
  vbo_.reset();
        
  set_size(width, ascender_ - descender_);
}
    
}  // namespace BlendInt