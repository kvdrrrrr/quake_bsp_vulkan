#ifndef BSPMAP_INCLUDED
#define BSPMAP_INCLUDED

#include "q3bsp/Q3BspRenderHelpers.hpp"

/*
 *  Base class for renderable bsp map. Future reference for other BSP format support.
 */

class BspMap
{
public:
    BspMap(bool bspValid) : m_bspValid(bspValid) {}
    virtual ~BspMap() {}

    virtual void Init() = 0;
    virtual void OnRender()        = 0; // perform rendering
    virtual void OnUpdate(const Math::Vector3f &cameraPosition) = 0; // update BSP visibility info for given camera position
    virtual void RebuildPipeline()          = 0; // rebuild Vulkan pipelines from scratch
    virtual std::string ThreadAndBspStats() = 0; // update BSP per-frame statistics and return a formatted string with thread workload

    virtual bool ClusterVisible(int cameraCluster, int testCluster) const   = 0;  // determine bsp cluster visibility
    virtual int  FindCameraLeaf(const Math::Vector3f &cameraPosition) const = 0;  // return bsp leaf index containing the camera
    virtual void CalculateVisibleFaces(int threadIndex, int cameraLeaf)     = 0;  // determine which bsp faces are visible

    // render helpers - extra flags + map statistics
    virtual void ToggleRenderFlag(int flag) = 0;
    inline bool  HasRenderFlag(int flag) const { return (m_renderFlags & flag) == flag; }
    inline bool  Valid() const { return m_bspValid; }
    inline const BspStats &GetMapStats() const { return m_mapStats; }
protected:
    int      m_renderFlags = 0;
    bool     m_bspValid;
    BspStats m_mapStats;
};

#endif