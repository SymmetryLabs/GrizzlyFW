#ifndef HEADER_GRIZZLY_TYPES
#define HEADER_GRIZZLY_TYPES

namespace GrizzlyLib
{
  template< class sElementFormat >
  class ElementBuffer;

  template< class sSrcFormat, class sDestFormat >
  class ElementConverter;

  template<class sSrc1Format, class sSrc2Format, class sDestFormat >
  class ElementInterpolator;

  template< class sSubObject >
  using SharedBufferPtr = std::shared_ptr< ElementBuffer<sSubObject> >;

  template< class sSubObject1, class sSubObject2 >
  using SharedConverterPtr = std::shared_ptr< ElementConverter<sSubObject1, sSubObject2> >;

  template< class sSubObject1, class sSubObject2, class sSubObject3>
  using SharedInterpolatorPtr = std::shared_ptr< ElementInterpolator<sSubObject1, sSubObject2, sSubObject3> >;

  template<class sFormat>
  using BufferPtr = typename ElementBuffer<sFormat>::BufferPtr;

  template<class sFormat>
  using BufferType = typename ElementBuffer<sFormat>::bufferType;

  template<class sSrcFormat, class sDestFormat>
  using ConverterPtr = typename ElementConverter<sSrcFormat,sDestFormat>::ConverterPtr;

  template<class sSrcFormat, class sDestFormat>
  using ConverterType = typename ElementConverter<sSrcFormat,sDestFormat>::bufferType;  

  typedef uint32_t ObjectId;
  typedef const std::string& ObjectType;
  typedef uint32_t ChannelId;

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

  template< class sFormat >
  class Format
  {

  };
};

#endif /* HEADER_GRIZZLY_TYPES */
