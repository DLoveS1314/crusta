#include <algorithm>
#include <iostream>
#include <cassert>

#include <Math/Constants.h>
#include <Vrui/Vrui.h>

BEGIN_CRUSTA


template <typename DataParam>
const FrameStamp CacheBuffer<DataParam>::
OLDEST_FRAMESTAMP(0);

template <typename DataParam>
CacheBuffer<DataParam>::
CacheBuffer() :
    frameStamp(OLDEST_FRAMESTAMP)
{
    state.grabbed = 0;
    state.valid   = 0;
    state.pinned  = 0;
}

template <typename DataParam>
const FrameStamp& CacheBuffer<DataParam>::
getFrameStamp() const
{
    return frameStamp;
}

template <typename DataParam>
DataParam& CacheBuffer<DataParam>::
getData()
{
    return this->data;
}

template <typename DataParam>
const DataParam& CacheBuffer<DataParam>::
getData() const
{
    return this->data;
}


template <typename DataParam>
CacheArrayBuffer<DataParam>::
~CacheArrayBuffer()
{
    delete[] this->data;
}


template <typename BufferParam>
IndexedBuffer<BufferParam>::
IndexedBuffer(DataIndex iIndex, BufferParam* iBuffer) :
    index(iIndex), buffer(iBuffer)
{
}

template <typename BufferParam>
bool IndexedBuffer<BufferParam>::
operator >(const IndexedBuffer& other) const
{
    if (buffer->state.valid == 0)
    {
        return other.buffer->state.valid==0 ?
               buffer->frameStamp > other.buffer->frameStamp : false;
    }
    else
    {
        return other.buffer->state.valid==0 ?
               true : buffer->frameStamp > other.buffer->frameStamp;
    }
}


template <typename BufferParam>
CacheUnit<BufferParam>::
CacheUnit() :
    pinUnpinLruDirty(true), touchResetLruDirty(true)
{
}

template <typename BufferParam>
CacheUnit<BufferParam>::
~CacheUnit()
{
    //deallocate all the cached buffers
    typedef typename BufferPtrMap::const_iterator iterator;
    for (iterator it=cached.begin(); it!=cached.end(); ++it)
        delete it->second;
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
clear()
{
    typedef typename BufferPtrMap::const_iterator iterator;
    for (iterator it=cached.begin(); it!=cached.end(); ++it)
        reset(it->second);

    pinUnpinLruDirty   = true;
    touchResetLruDirty = true;
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
init(const std::string& iName, int size)
{
    name = iName;

    //fill the cache with buffers with no valid content
    for (int i=0; i<size; ++i)
    {
        BufferParam* buffer = new BufferParam;
        initData(buffer->getData());
        DataIndex dummyIndex(~0, TreeIndex(~0,~0,~0,i));
        cached.insert(typename BufferPtrMap::value_type(dummyIndex, buffer));
    }
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
initData(typename BufferParam::DataType&)
{
}


template <typename BufferParam>
bool CacheUnit<BufferParam>::
isValid(const BufferParam* const buffer) const
{
    return buffer->state.valid==1;
}

template <typename BufferParam>
bool CacheUnit<BufferParam>::
isPinned(const BufferParam* const buffer) const
{
    return buffer->state.pinned>0;
}

template <typename BufferParam>
bool CacheUnit<BufferParam>::
isGrabbed(const BufferParam* const buffer) const
{
    return buffer->state.grabbed==1;
}

template <typename BufferParam>
bool CacheUnit<BufferParam>::
isCurrent(const BufferParam* const buffer) const
{
    return isValid(buffer) && buffer->frameStamp == CURRENT_FRAME;
}


template <typename BufferParam>
void CacheUnit<BufferParam>::
touch(BufferParam* buffer)
{
    buffer->frameStamp  = CURRENT_FRAME;
    buffer->state.valid = 1;
    if (buffer->state.grabbed == 0)
        touchResetLruDirty = true;
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
reset(BufferParam* buffer)
{
    buffer->state.valid  = 0;
    buffer->state.pinned = 0;
    buffer->frameStamp   = BufferParam::OLDEST_FRAMESTAMP;
    if (buffer->state.grabbed == 0)
        pinUnpinLruDirty = true;
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
reset(int numBuffers)
{
    //make sure we have an LRU to manipulate
    refreshLru();

    //reset numBuffers starting at the tail of the LRU
    typename IndexedBuffers::reverse_iterator lit = lruCached.rbegin();
    for (int i=0; i<numBuffers && lit!=lruCached.rend(); ++i, ++lit)
        reset(lit->buffer);
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
pin(BufferParam* buffer)
{
    ++buffer->state.pinned;
    if (buffer->state.pinned==0)
        --buffer->state.pinned;
    if (buffer->state.grabbed == 0)
        pinUnpinLruDirty = true;
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
unpin(BufferParam* buffer)
{
    if (buffer->state.pinned!=0)
        --buffer->state.pinned;
    if (buffer->state.grabbed == 0)
        pinUnpinLruDirty = true;
}


template <typename BufferParam>
BufferParam* CacheUnit<BufferParam>::
find(const DataIndex& index) const
{
    typename BufferPtrMap::const_iterator it = cached.find(index);
    if (it!=cached.end())
    {
CRUSTA_DEBUG(20, CRUSTA_DEBUG_OUT <<
name << "Cache" << cached.size() << "::find: found " <<
(isPinned(it->second) ? '*' : ' ') << index.med_str() << "\n";)
        return it->second;
    }
    else
    {
CRUSTA_DEBUG(19, CRUSTA_DEBUG_OUT <<
name << "Cache" << cached.size() << "::find: missed " << index.med_str() <<
"\n";)
        return NULL;
    }
}

template <typename BufferParam>
BufferParam* CacheUnit<BufferParam>::
grabBuffer(bool grabCurrent)
{
//- try to swap from the LRU
    refreshLru();
CRUSTA_DEBUG(18, printCache();)

    //check the tail of the LRU sequence for valid buffers
    IndexedBuffer<BufferParam> lruBuf(DataIndex(), NULL);

    if (!lruCached.empty())
    {
        //make sure the tail suffices the conditions
        lruBuf = lruCached.back();
        if (!grabCurrent && isCurrent(lruBuf.buffer))
            lruBuf.buffer = NULL;
    }

    //if we found a valid buffer remove it from the map and the lru
    if (lruBuf.buffer != NULL)
    {
CRUSTA_DEBUG(15, CRUSTA_DEBUG_OUT <<
name << "Cache" << cached.size() << "::grabbed " << lruBuf.index.med_str() <<
"\n";)
        assert(cached.find(lruBuf.index)!=cached.end());
        lruBuf.buffer->state.grabbed = 1;
        cached.erase(lruBuf.index);
        lruCached.pop_back();
    }
    else
    {
CRUSTA_DEBUG(11, CRUSTA_DEBUG_OUT <<
name << "Cache" << cached.size() << ":: unable to provide buffer\n";)
    }

    return lruBuf.buffer;
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
releaseBuffer(const DataIndex& index, BufferParam* buffer)
{
    //silently ignore buffers that have not been grabbed
    if (buffer->state.grabbed == 0)
    {
        //validate the buffer
        buffer->state.valid = 1;
        buffer->frameStamp  = CURRENT_FRAME;
        return;
    }

    assert(cached.find(index)==cached.end());

CRUSTA_DEBUG(15, CRUSTA_DEBUG_OUT <<
name << "Cache" << cached.size() << "::released " << index.med_str() << "\n";)

    cached.insert(typename BufferPtrMap::value_type(index, buffer));
    assert(cached.find(index)!=cached.end());

    //validate the buffer
    buffer->state.grabbed = 0;
    buffer->state.valid   = 1;
    buffer->frameStamp    = CURRENT_FRAME;

    //since we manipulated the cache, the LRU will have to be recomputed
    pinUnpinLruDirty = true;
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
printCache()
{
#if CRUSTA_ENABLE_DEBUG
if (name != std::string("MainLayerf"))
    return;

    CRUSTA_DEBUG_OUT << "print" << name << "Cache" << cached.size() <<
                        ": frame " << CURRENT_FRAME << "\n";
    IndexedBuffers lru;
    for (typename BufferPtrMap::const_iterator it=cached.begin();
         it!=cached.end(); ++it)
    {
        if (isPinned(it->second))
        {
            CRUSTA_DEBUG_OUT << (it->second->state.valid==0 ? '#' : ' ') <<
                                it->first.med_str() << " " <<
                                it->second->frameStamp << " ";
        }
        else
        {
            lru.push_back(IndexedBuffer<BufferParam>(it->first, it->second));
        }
    }
    CRUSTA_DEBUG_OUT << "\n-------\n";
    std::sort(lru.begin(), lru.end(),
              std::greater< IndexedBuffer<BufferParam> >());
    for (typename IndexedBuffers::const_iterator it=lru.begin();
         it!=lru.end(); ++it)
    {
        CRUSTA_DEBUG_OUT << (it->buffer->state.valid==0 ? '#' : ' ') <<
                            it->index.med_str() << " " <<
                            it->buffer->frameStamp << " ";
    }
    CRUSTA_DEBUG_OUT << "\n\n";
#endif //CRUSTA_ENABLE_DEBUG
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
printLru(const char* cause)
{
#if CRUSTA_ENABLE_DEBUG
if (name != std::string("GpuGeometry"))
    return;

    CRUSTA_DEBUG_OUT << "Refresh" << name << "LRU" << cause << cached.size() <<
                        ": frame " << CURRENT_FRAME << "\n";
    for (typename IndexedBuffers::const_iterator it=lruCached.begin();
         it!=lruCached.end(); ++it)
    {
        CRUSTA_DEBUG_OUT << (it->buffer->state.valid==0 ? '#' : ' ') <<
                            it->index.med_str() << " " <<
                            it->buffer->frameStamp << " ";
    }
    CRUSTA_DEBUG_OUT << "\n\n";
#endif //CRUSTA_ENABLE_DEBUG
}

template <typename BufferParam>
void CacheUnit<BufferParam>::
refreshLru()
{
    if (pinUnpinLruDirty)
    {
        //repopulate
        lruCached.clear();
        for (typename BufferPtrMap::const_iterator it=cached.begin();
             it!=cached.end(); ++it)
        {
            if (!isPinned(it->second))
            {
                lruCached.push_back(
                    IndexedBuffer<BufferParam>(it->first, it->second));
            }
        }
        //resort
        std::sort(lruCached.begin(), lruCached.end(),
                  std::greater< IndexedBuffer<BufferParam> >());
CRUSTA_DEBUG(17, printLru("Pin");)
    }
    else if (touchResetLruDirty)
    {
        //resort only
        std::sort(lruCached.begin(), lruCached.end(),
                  std::greater< IndexedBuffer<BufferParam> >());
CRUSTA_DEBUG(17, printLru("Touch");)
    }

    //clear the dirty flags
    pinUnpinLruDirty   = false;
    touchResetLruDirty = false;
}


END_CRUSTA
