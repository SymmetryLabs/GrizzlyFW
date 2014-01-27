#ifndef HEADER_COLOR_INTERPOLATORS 
#define HEADER_COLOR_INTERPOLATORS 

#include <string>
#include "grizzly_types.h"

namespace GrizzlyLib
{
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
    
  
  /* ElementInterpolator template function implementation macros */
  #define elementInterpolatorT(_src1_format, _src2_format, _dest_format) ElementInterpolator<_src1_format, _src2_format, _dest_format>
  #define elementInterpolatorF(_function, _src1_format, _src2_format, _dest_format) elementInterpolatorT(_src1_format, _src2_format, _dest_format)::_function
  #define def_interpolateElement(_src1_format, _src2_format, _dest_format) template<> inline void elementInterpolatorF(interpolateElement, _src1_format, _src2_format, _dest_format)
  #define def_interpolateElements(_src1_format, _src2_format, _dest_format) template<> void elementInterpolatorF(interpolateElements, _src1_format, _src2_format, _dest_format)
  #define def_interpolatorType(_src1_format, _src2_format, _dest_format) template<> inline const std::string& elementInterpolatorF(interpolatorType, _src1_format, _src2_format, _dest_format)

  const std::string ts_rgbrgb2rgb = std::string("rgbrgb2rgb");

  def_interpolatorType(RGBFormat, RGBFormat, RGBFormat)() {return ts_rgbrgb2rgb; }


  def_interpolateElement(RGBFormat, RGBFormat, RGBFormat)(RGBFormat *src1, RGBFormat *src2, RGBFormat *dest, float ratio)
  {
      dest->R = (uint8_t) (src1->R + ratio * (src2->R - src1->R));
      dest->G = (uint8_t) (src1->G + ratio * (src2->G - src1->G));
      dest->B = (uint8_t) (src1->B + ratio * (src2->B - src1->B));
  }
};
// template<class sFormat>
// void elementInterpolatorF(interpolateElements, sFormat, sFormat, sFormat)(uint32_t num_elements)
// {
//  elementInterpolatorT(sFormat, sFormat, sFormat)::interpolateElement(src1, src2, dest);
// }
#endif /* HEADER_COLOR_INTERPOLATORS */