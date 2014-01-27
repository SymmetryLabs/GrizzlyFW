#ifndef HEADER_GRIZZLY_TYPES
#define HEADER_GRIZZLY_TYPES

#include <memory>
#include <unordered_map>
#include <map>

namespace GrizzlyLib
{
  typedef std::shared_ptr<void> UncastPtr;
  
  class FrameGroup;
  typedef std::shared_ptr<FrameGroup> FramePtr;
  
  class ObjectManager;
  typedef uint32_t ObjectId;
  typedef uint32_t ChannelId;
  typedef const std::string& ObjectType;

  struct ObjectPtr {
            ObjectId id;
            ObjectType type;
            UncastPtr mem_ptr;
            UncastPtr obj_ptr;
						
						ObjectPtr(ObjectId id_, ObjectType type_, UncastPtr mem_ptr_, UncastPtr obj_ptr_) : id(id_), type(type_), mem_ptr(mem_ptr_), obj_ptr(obj_ptr_) {} ;
    };

  typedef std::unordered_map<ObjectId, std::shared_ptr<ObjectPtr> > ObjectPtrMap;
  typedef std::unordered_map<ChannelId , std::pair<std::string, ObjectId> > PendingBufferMap;

  template< class sElementFormat >
  class ElementBuffer;

  template< class sSubObject >
  using SharedBufferPtr = std::shared_ptr< ElementBuffer<sSubObject> >;

  template<class sFormat>
  using BufferPtr = typename ElementBuffer<sFormat>::BufferPtr;

  template<class sFormat>
  using BufferType = typename ElementBuffer<sFormat>::bufferType;

  /*
  template< class sSrcFormat, class sDestFormat >
  class ElementConverter;

  template< class sSubObject1, class sSubObject2 >
  using SharedConverterPtr = std::shared_ptr< ElementConverter<sSubObject1, sSubObject2> >;

  template<class sSrcFormat, class sDestFormat>
  using ConverterPtr = typename ElementConverter<sSrcFormat,sDestFormat>::ConverterPtr;

  template<class sSrcFormat, class sDestFormat>
  using ConverterType = typename ElementConverter<sSrcFormat,sDestFormat>::bufferType;  
  */

  /*
  template<class sSrc1Format, class sSrc2Format, class sDestFormat >
  class ElementInterpolator;

  template< class sSubObject1, class sSubObject2, class sSubObject3>
  using SharedInterpolatorPtr = std::shared_ptr< ElementInterpolator<sSubObject1, sSubObject2, sSubObject3> >;
  */


  struct ARGBFormat
  {
    uint8_t A;
    uint8_t R;
    uint8_t G;
    uint8_t B;
  };

  struct RGBFormat
  {
    uint8_t R;
    uint8_t G;
    uint8_t B;
  };

  struct XYZFormat
  {
    float X;
    float Y;
    float Z;
  };

  struct SlowXYZFormat : XYZFormat {};

  struct DMAFormat
  {
    uint32_t R[4];
    uint32_t G[4];
    uint32_t B[4];
  };

  struct SlowDMAFormat {
    uint16_t R[8];
    uint16_t G[8];
    uint16_t B[8];
  };
};

#endif /* HEADER_GRIZZLY_TYPES */
