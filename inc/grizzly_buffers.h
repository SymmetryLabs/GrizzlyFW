#ifndef HEADER_COLOR_FORMATS
#define HEADER_COLOR_FORMATS

#include <string>
#include <vector>
#include "grizzly_types.h"
#include "grizzly_manager.h"

namespace GrizzlyLib
{
  template< class sElementFormat >
  class ElementBuffer
  {
  public:
    typedef sElementFormat* iterator;
    typedef const sElementFormat* const_iterator;
    typedef SharedBufferPtr<sElementFormat> BufferPtr;

  private:
    iterator first;
    iterator last;

  public:
    static ObjectType bufferType();

    iterator begin() { return first; }
    iterator end() { return last; }

    ElementBuffer(iterator _first, iterator _last) : first(_first), last(_last)
    {

    };

    template< class cSrcBuffer >
    static BufferPtr createBufferFromSource(cSrcBuffer *buf_ptr, uint32_t offset, uint32_t length);

    static BufferPtr createBufferFromIterators(iterator src_first, iterator src_last) {
      BufferPtr new_buf( new ElementBuffer<sElementFormat>(src_first, src_last) );

      return new_buf;
    };
  };

  #define elementBufferT(_format) ElementBuffer<_format>
  #define elementBufferF(_function, _format) elementBufferT(_format)::_function
  #define def_bufferType(_format) template<> const std::string&  elementBufferF(bufferType, _format)

  // const std::string ts_ = std::string("");
  const std::string ts_argb = std::string("argb");
  const std::string ts_rgb = std::string("rgb");
  const std::string ts_xyz = std::string("xyz");
  const std::string ts_s_xyz = std::string("s_xyz");
  const std::string ts_dma = std::string("dma");
  const std::string ts_s_dma = std::string("s_dma");

  def_bufferType(ARGBFormat)()  { return ts_argb; }
  def_bufferType(RGBFormat)() { return ts_rgb; }
  def_bufferType(XYZFormat)() { return ts_xyz; }
  def_bufferType(SlowXYZFormat)() { return ts_s_xyz; }
  def_bufferType(DMAFormat)() { return ts_dma; }
  def_bufferType(SlowDMAFormat)() { return ts_s_dma; }

  template< class sFormat >
  struct BufferFactory
  {
    static ObjectId allocateBuffer(ObjectManager* objman, UncastPtr block_ptr, const void* start, const void* finish)
    {
      auto start_ptr = (typename ElementBuffer<sFormat>::iterator) (start);
      auto finish_ptr = (typename ElementBuffer<sFormat>::iterator) (finish);
      auto obj_ptr = std::make_shared< ElementBuffer<sFormat> >(start_ptr, finish_ptr);
      return objman->allocateObject(ElementBuffer<sFormat>::bufferType(), block_ptr, obj_ptr);
    }

    static ObjectId allocateBuffer(ObjectManager* objman, uint32_t size)
    {
        auto block_ptr = std::make_shared< std::vector<sFormat> >(size);
        auto obj_ptr = std::make_shared< ElementBuffer<sFormat> >(&*block_ptr->begin(), &*block_ptr->end());
        return objman->allocateObject(ElementBuffer<sFormat>::bufferType(), block_ptr, obj_ptr);
    }

    static BufferPtr<sFormat> getBuffer(ObjectManager* objman, ObjectId obj_id)
    {
        auto obj_ptr = objman->getObject(obj_id, ElementBuffer<sFormat>::bufferType());
        return std::static_pointer_cast< ElementBuffer<sFormat> >(obj_ptr);
    }
  };

};
#endif /* HEADER_COLOR_FORMATS */
