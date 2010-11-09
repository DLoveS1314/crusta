#ifndef _Triacontahedron_H_
#define _Triacontahedron_H_

#include <crusta/Polyhedron.h>

BEGIN_CRUSTA

/**
    Provides the basic data to build a rhombic Triacontahedron
*/
class Triacontahedron : public Polyhedron
{
//- inherited from Polyhedron
public:
    Triacontahedron(Scope::Scalar sphereRadius=1.0);

    virtual uint  getNumPatches();
    virtual Scope getScope(uint patchId);
    virtual void  getConnectivity(uint patchId, Connectivity connectivity[4]);

protected:
    Scope::Scalar radius;
};

END_CRUSTA

#endif //_Triacontahedron_H_
