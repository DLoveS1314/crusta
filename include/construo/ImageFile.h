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

#ifndef _ImageFile_H_
#define _ImageFile_H_

#include <string>

#include <crusta/basics.h>

BEGIN_CRUSTA

template <typename PixelParam>
struct Nodata;

template <typename PixelParam>
class ImageFileBase
{
public:
    ///data type for pixel values stored in images
    typedef PixelParam Pixel;

    ImageFileBase();
    virtual ~ImageFileBase();

    ///set the uniform scale of the pixel values
    void setPixelScale(double scale);
    ///retrieve the uniform scale fo the pixel values
    double getPixelScale() const;
    ///set a nodata value to use
    virtual void setNodata(const std::string& nodataString) = 0;
    ///retrieve the nodata value
    const Nodata<PixelParam>& getNodata() const;
    ///returns image size
    const int* getSize() const;
    ///reads a rectangle of pixel data into the given buffer
    virtual void readRectangle(const int rectOrigin[2], const int rectSize[2],
                               Pixel* rectBuffer) const = 0;

protected:
    /** the uniform scale of the values of the image (in particular for DEMs
        that would be the elevation resolution in meters) */
    double pixelScale;
    ///the value corresponding to "no data"
    Nodata<PixelParam> nodata;
    ///size of the image in pixels (width x height)
    int size[2];
};

template <typename PixelParam>
class ImageFile : public ImageFileBase<PixelParam>
{
};

END_CRUSTA

#include <construo/ImageFile.hpp>

#endif //_ImageFile_H_
