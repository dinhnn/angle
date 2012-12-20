//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexDataManager.h: Defines the IndexDataManager, a class that
// runs the Buffer translation process for index buffers.

#ifndef LIBGLESV2_INDEXDATAMANAGER_H_
#define LIBGLESV2_INDEXDATAMANAGER_H_

#include <vector>
#include <cstddef>

#define GL_APICALL
#include <GLES2/gl2.h>

#include "libGLESv2/Context.h"

namespace
{
    enum { INITIAL_INDEX_BUFFER_SIZE = 4096 * sizeof(GLuint) };
}

namespace rx
{

struct TranslatedIndexData
{
    UINT minIndex;
    UINT maxIndex;
    UINT startIndex;
};

class IndexBuffer
{
  public:
    IndexBuffer(rx::Renderer9 *renderer, UINT size, D3DFORMAT format);
    virtual ~IndexBuffer();

    UINT size() const { return mBufferSize; }
    virtual void *map(UINT requiredSpace, UINT *offset) = 0;
    void unmap();
    virtual void reserveSpace(UINT requiredSpace, GLenum type) = 0;

    IDirect3DIndexBuffer9 *getBuffer() const;
    unsigned int getSerial() const;

  protected:
    rx::Renderer9 *const mRenderer;   // D3D9_REPLACE

    IDirect3DIndexBuffer9 *mIndexBuffer;
    UINT mBufferSize;

    unsigned int mSerial;
    static unsigned int issueSerial();
    static unsigned int mCurrentSerial;

  private:
    DISALLOW_COPY_AND_ASSIGN(IndexBuffer);
};

class StreamingIndexBuffer : public IndexBuffer
{
  public:
    StreamingIndexBuffer(rx::Renderer9 *renderer, UINT initialSize, D3DFORMAT format);
    ~StreamingIndexBuffer();

    virtual void *map(UINT requiredSpace, UINT *offset);
    virtual void reserveSpace(UINT requiredSpace, GLenum type);

  private:
    UINT mWritePosition;
};

class StaticIndexBuffer : public IndexBuffer
{
  public:
    explicit StaticIndexBuffer(rx::Renderer9 *renderer);
    ~StaticIndexBuffer();

    virtual void *map(UINT requiredSpace, UINT *offset);
    virtual void reserveSpace(UINT requiredSpace, GLenum type);

    bool lookupType(GLenum type);
    UINT lookupRange(intptr_t offset, GLsizei count, UINT *minIndex, UINT *maxIndex);   // Returns the offset into the index buffer, or -1 if not found
    void addRange(intptr_t offset, GLsizei count, UINT minIndex, UINT maxIndex, UINT streamOffset);

  private:
    GLenum mCacheType;

    struct IndexRange
    {
        intptr_t offset;
        GLsizei count;

        bool operator<(const IndexRange& rhs) const
        {
            if (offset != rhs.offset)
            {
                return offset < rhs.offset;
            }
            if (count != rhs.count)
            {
                return count < rhs.count;
            }
            return false;
        }
    };

    struct IndexResult
    {
        UINT minIndex;
        UINT maxIndex;
        UINT streamOffset;
    };

    std::map<IndexRange, IndexResult> mCache;
};

class IndexDataManager
{
  public:
    IndexDataManager(rx::Renderer9 *renderer);
    virtual ~IndexDataManager();

    GLenum prepareIndexData(GLenum type, GLsizei count, gl::Buffer *arrayElementBuffer, const GLvoid *indices, TranslatedIndexData *translated, IDirect3DIndexBuffer9 **indexBuffer, unsigned int *serial);
    StaticIndexBuffer *getCountingIndices(GLsizei count);

  private:
    DISALLOW_COPY_AND_ASSIGN(IndexDataManager);

    std::size_t typeSize(GLenum type) const;
    std::size_t indexSize(D3DFORMAT format) const;

    rx::Renderer9 *const mRenderer;   // D3D9_REPLACE

    StreamingIndexBuffer *mStreamingBufferShort;
    StreamingIndexBuffer *mStreamingBufferInt;
    StaticIndexBuffer *mCountingBuffer;
};

}

#endif   // LIBGLESV2_INDEXDATAMANAGER_H_