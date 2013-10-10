/*
 * This file is part of BILO (Blender-like Interface Library in
 * OpenGL).
 *
 * BILO (Blender-like Interface Library in OpenGL) is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * BILO (Blender-like Interface Library in OpenGL) is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BILO.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#include <BILO/Image.hpp>
#include <OpenImageIO/imageio.h>

OIIO_NAMESPACE_USING

namespace BILO {

	Image::Image ()
	: m_width(0), m_height(0), m_channels(0)
	{

	}

	Image::~Image()
	{

	}

	bool Image::read (const String& filename)
	{
		ImageInput *in = ImageInput::open (ConvertFromString(filename));

		if (! in)
			return false;

		const ImageSpec &spec = in->spec();

		m_width = spec.width;
		m_height = spec.height;
		m_channels = spec.nchannels;
		m_pixels.resize (m_width * m_height * m_channels);
		in->read_image (TypeDesc::UINT8, &m_pixels[0]);
		in->close ();

		delete in;

		return true;
	}

}
