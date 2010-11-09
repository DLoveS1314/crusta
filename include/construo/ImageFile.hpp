///\todo fix GPL

/***********************************************************************
ImageFile - Base class to represent image files for simple reading of
individual tiles
Copyright (c) 2005-2008 Oliver Kreylos

This file is part of the DEM processing and visualization package.

The DEM processing and visualization package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The DEM processing and visualization package is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the DEM processing and visualization package; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

BEGIN_CRUSTA

template <class PixelParam>
inline
ImageFile<PixelParam>::
ImageFile() :
    pixelScale(1.0)
{
    size[0] = size[1] = 0;
}

template <class PixelParam>
inline
ImageFile<PixelParam>::
~ImageFile()
{
}

template <class PixelParam>
inline void
ImageFile<PixelParam>::
setPixelScale(double scale)
{
    pixelScale = scale;
}

template <class PixelParam>
inline double
ImageFile<PixelParam>::
getPixelScale() const
{
    return pixelScale;
}

template <class PixelParam>
inline
const ImageFile<PixelParam>::Traits::Nodata& ImageFile<PixelParam>::
getNodata() const
{
    return nodata;
}

template <class PixelParam>
inline
void ImageFile<PixelParam>::
setNodata(const Traits::Nodata& nodataValue)
{
    nodata = nodataValue;
}

template <class PixelParam>
inline
const int* ImageFile<PixelParam>::
getSize() const
{
    return size;
}

END_CRUSTA
