#include <crusta/map/Polyline.h>

#include <cassert>

#include <Vrui/Vrui.h>

#include <crusta/Crusta.h>


BEGIN_CRUSTA


Polyline::
Polyline(Crusta* iCrusta) :
    Shape(iCrusta)
{
}


void Polyline::
recomputeCoords(ControlPointHandle cur)
{
    FrameStamp curStamp = Vrui::getApplicationTime();

    assert(cur != controlPoints.end());

    Point3 prevP, curP;
    ControlPointHandle prev = cur;
    if (prev != controlPoints.begin())
        --prev;
    else
    {
        prev->coord = 0.0;
        ++cur;
    }
    prevP = crusta->mapToScaledGlobe(prev->pos);
    for (; cur!=controlPoints.end(); ++prev, ++cur, prevP=curP)
    {
        curP = crusta->mapToScaledGlobe(cur->pos);
        cur->coord  = prev->coord + Geometry::dist(prevP, curP);
        cur->age    = curStamp;
    }
}

void Polyline::
setControlPoints(const Point3s& newControlPoints)
{
    Shape::setControlPoints(newControlPoints);
    recomputeCoords(controlPoints.begin());
}

Shape::ControlId Polyline::
addControlPoint(const Point3& pos, End end)
{
    Shape::ControlId ret = Shape::addControlPoint(pos, end);
    assert(ret.isValid());
    recomputeCoords(ret.handle);
    return ret;
}

void Polyline::
moveControlPoint(const ControlId& id, const Point3& pos)
{
    Shape::moveControlPoint(id, pos);
    recomputeCoords(id.handle);
}

void Polyline::
removeControlPoint(const ControlId& id)
{
    assert(id.isValid());

    if (id.handle==controlPoints.begin() || id.handle==--controlPoints.end())
    {
        Shape::removeControlPoint(id);
    }
    else
    {
        ControlPointHandle next = id.handle;
        ++next;
        Shape::removeControlPoint(id);
        recomputeCoords(next);
    }
}

Shape::ControlId Polyline::
refine(const ControlId& id, const Point3& pos)
{
    Shape::ControlId ret = Shape::refine(id, pos);
    assert(ret.isValid());
    recomputeCoords(ret.handle);
    return ret;
}

END_CRUSTA
