#ifndef HEADER_GRIZZLYLIB
#define HEADER_GRIZZLYLIB

extern "C" {
	#include "stm32f4xx.h"
}
#include <queue>
#include <map>

#include <memory>
//#include <cassert>
//#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_map>


namespace GrizzyLib {
  
  template< template <typename> class sObject, class sSubObject >
  using SharedBufferPtr = std::shared_ptr< sObject<sSubObject> >;

  template< template <typename, typename> class sObject, class sSubObject1, class sSubObject2>
  using SharedConverterPtr = std::shared_ptr< sObject<sSubObject1, sSubObject2> >;

  template< template <typename, typename, typename> class sObject, class sSubObject1, class sSubObject2, class sSubObject3>
  using SharedInterpolatorPtr = std::shared_ptr< sObject<sSubObject1, sSubObject2, sSubObject3> >;

  typedef uint32_t ObjectId;
  typedef const std::string& ObjectType;
  typedef uint32_t ChannelId;

  template< class sElementFormat >
  class ElementBuffer
  {
  public:
    typedef sElementFormat* iterator;
    typedef const sElementFormat* const_iterator;
    typedef SharedBufferPtr<ElementBuffer, sElementFormat> BufferPtr;

  private:
    iterator first;
    iterator last;

  public:
    static const std::string& bufferType();

    iterator begin() { return first; }
    iterator end() { return last; }

    ElementBuffer(iterator _first, iterator _last) : first(_first), last(_last)
    {

    };

    ~ElementBuffer()
    {

    };  

    template< class cSrcBuffer >
    static BufferPtr createBufferFromSource(cSrcBuffer *buf_ptr, uint32_t offset, uint32_t length);

    static BufferPtr createBufferFromIterators(iterator src_first, iterator src_last) {
      BufferPtr new_buf( new ElementBuffer<sElementFormat>(src_first, src_last) );

      return new_buf;
    };
  };

  template<class sFormat>
  using BufferPtr = typename ElementBuffer<sFormat>::BufferPtr;

  template<class sFormat>
  using BufferType = typename ElementBuffer<sFormat>::bufferType;

  template< class sSrcFormat, class sDestFormat >
  class ElementConverter
  {
  private:
    typename ElementBuffer<sSrcFormat>::BufferPtr source;
    typename ElementBuffer<sSrcFormat>::iterator source_itr;

    typename ElementBuffer<sDestFormat>::BufferPtr destination;
    typename ElementBuffer<sDestFormat>::iterator destination_itr;
    
  public:
    typedef SharedConverterPtr<ElementConverter, sSrcFormat, sDestFormat>  ConverterPtr;

    static inline bool compareElement(sSrcFormat *src, sDestFormat *dest);
    static inline void convertElement(sSrcFormat *src, sDestFormat *dest);
    static inline const std::string& converterType();

    typename ElementBuffer<sSrcFormat>::iterator lastElement(uint32_t num_elements)
    {
      typename ElementBuffer<sSrcFormat>::iterator ite = source_itr + num_elements;

      if (ite > source->end())
      {
        ite = source->end();
      }

      if ((destination_itr + num_elements) > destination->end())
      {
        ite = source_itr;
        typename ElementBuffer<sDestFormat>::iterator itd = destination_itr;
        while (itd != destination->end())
        {
          ite++;
          itd++;
        }
      }

      return ite;
    }

    void convertElements(uint32_t num_elements)
    {
      typename ElementBuffer<sSrcFormat>::iterator ite = lastElement(num_elements);

      while (source_itr != ite)
      {
        ElementConverter<sSrcFormat, sDestFormat>::convertElement(source_itr++, destination_itr++);
      }
    }

    /* DMA converter needs channel number. uint32_t as placeholder for ptr/other data */
    void convertElements(uint32_t num_elements, uint32_t channel_num)
    {
      convertElements(num_elements);
    }

    /* ===== Static Members ===== */
    static ConverterPtr createConverter(typename ElementBuffer<sSrcFormat>::BufferPtr _source, \
                                        typename ElementBuffer<sDestFormat>::BufferPtr _destination)
    {
      ConverterPtr new_conv( new ElementConverter<sSrcFormat, sDestFormat> );
      new_conv->source = _source;
      new_conv->source_itr = _source->begin();
      new_conv->destination = _destination;
      new_conv->destination_itr = _destination->begin();
      return new_conv;
    }

    ElementConverter()
    {
      // cout << "ElementConverter constructed" << endl;
    }

    ~ElementConverter()
    {
      // cout << "ElementConverter destructed" << endl;
    }
  };

  template<class sSrcFormat, class sDestFormat>
  using ConverterPtr = typename ElementConverter<sSrcFormat,sDestFormat>::ConverterPtr;

  template<class sSrc1Format, class sSrc2Format, class sDestFormat>
  class ElementInterpolator
  {
  private:
    typename ElementBuffer<sSrc1Format>::BufferPtr source1;
    typename ElementBuffer<sSrc1Format>::iterator source1_itr;

    typename ElementBuffer<sSrc2Format>::BufferPtr source2;
    typename ElementBuffer<sSrc2Format>::iterator source2_itr;

    typename ElementBuffer<sDestFormat>::BufferPtr destination;
    typename ElementBuffer<sDestFormat>::iterator destination_itr;

    typename ElementConverter<sSrc2Format, sSrc1Format>::ConverterPtr source_converter;
    typename ElementConverter<sSrc1Format, sDestFormat>::ConverterPtr destination_converter;

  public:
    typedef SharedInterpolatorPtr<ElementInterpolator, sSrc1Format, sSrc2Format, sDestFormat>  InterpolatorPtr;

    /* ===== Function Definitions ===== */
    static inline void interpolateElement(sSrc1Format *src1, sSrc2Format *src2, sDestFormat *dest, float ratio);
    static inline const std::string& interpolatorType();

    /* ===== Utility Functions ===== */
    inline bool checkBuffers() { return (source1 != 0) && (source2 != 0) && (destination !=0); }

    typename ElementBuffer<sSrc1Format>::iterator lastElement(uint32_t num_elements)
    {
      typename ElementBuffer<sSrc1Format>::iterator ite = source1_itr + num_elements;

      if (ite > source1->end())
      {
        ite = source1->end();
      }

      if ((destination_itr + num_elements) > destination->end() || (source2_itr + num_elements) > source2->end())
      {
        typename ElementBuffer<sSrc2Format>::iterator ite2 = source2_itr;
        typename ElementBuffer<sDestFormat>::iterator itd = destination_itr;
        ite = source1_itr;

        while (itd != destination->end() && ite2 != source2->end() && ite != source1->end())
        {
          ite++;
          ite2++;
          itd++;
        }
      }

      return ite;
    }

    /* ===== Worker Functions ===== */
    void setSources(BufferPtr<sSrc1Format> src1, BufferPtr<sSrc2Format> src2)
    {
      // if (ElementBufferer<sSrc1Format>::bufferType != )
      source1 = src1;
      source2 = src2;
      source1_itr = src1->begin();
      source2_itr = src2->begin();
    }

    void setDestination(BufferPtr<sDestFormat> dest)
    {
      destination = dest;
      destination_itr = dest->begin();
    }

    void interpolateElements(uint32_t num_elements)
    {
      auto ite = lastElement(num_elements);
      
      float ratio = .5f;
      while (source1_itr != ite)
      {
        ElementInterpolator<sSrc1Format, sSrc2Format, sDestFormat>::interpolateElement(source1_itr++, source2_itr++, destination_itr++, ratio);
      }
    }

    ElementInterpolator()
    {
      // cout << "ElementInterpolator constructed" << endl;
    };

    ~ElementInterpolator()
    {
      // cout << "ElementInterpolator destructed" << endl;
    };
  };
	
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
	#define elementBufferT(_format) ElementBuffer<_format>
#define elementBufferF(_function, _format) elementBufferT(_format)::_function
#define def_bufferType(_format) template<> const std::string&  elementBufferF(bufferType, _format)

/* ElementConverter template function implementation macros */
#define elementConverterT(_src_format, _dest_format) ElementConverter<_src_format, _dest_format>
#define elementConverterF(_function, _src_format, _dest_format) elementConverterT(_src_format, _dest_format)::_function
#define def_convertElement(_src_format, _dest_format) template<> inline void elementConverterF(convertElement, _src_format, _dest_format)
#define def_convertElements(_src_format, _dest_format) template<> void elementConverterF(convertElements, _src_format, _dest_format)
#define def_converterType(_src_format, _dest_format) template<> inline const std::string& elementConverterF(converterType, _src_format, _dest_format)

/* ElementInterpolator template function implementation macros */
#define elementInterpolatorT(_src1_format, _src2_format, _dest_format) ElementInterpolator<_src1_format, _src2_format, _dest_format>
#define elementInterpolatorF(_function, _src1_format, _src2_format, _dest_format) elementInterpolatorT(_src1_format, _src2_format, _dest_format)::_function
#define def_interpolateElement(_src1_format, _src2_format, _dest_format) template<> inline void elementInterpolatorF(interpolateElement, _src1_format, _src2_format, _dest_format)
#define def_interpolateElements(_src1_format, _src2_format, _dest_format) template<> void elementInterpolatorF(interpolateElements, _src1_format, _src2_format, _dest_format)
#define def_interpolatorType(_src1_format, _src2_format, _dest_format) template<> inline const std::string& elementInterpolatorF(interpolatorType, _src1_format, _src2_format, _dest_format)

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


  
  #include "color_formats.hpp"
  #include "color_converters.hpp"
  #include "color_interpolators.hpp"

  #include "frame_queue.hpp"
  #include "obj_manager.hpp"
};

#endif /* HEADER_GRIZZLYLIB */
