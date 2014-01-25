#ifndef HEADER_HELPERS
#define HEADER_HELPERS

#define elementBufferT(_format) ElementBuffer<_format>
#define elementBufferF(_function, _format) elementBufferT(_format)::_function
#define def_bufferType(_format) template <> inline const std::string& elementBufferF(bufferType, _format)

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

/* Allocates sFormat[length] and wraps it in ElementBuffer<sFormat> */
template< class sFormat >
typename ElementBuffer<sFormat>::BufferPtr createEmptyBuffer(uint32_t length)
{
  sFormat* format = new sFormat[length];
  return ElementBuffer<sFormat>::createBufferFromIterators(&format[0], &format[length]);
}

template< class sFormat, typename sType, typename sNewType>
void printBufferElements(typename ElementBuffer<sFormat>::iterator it, typename ElementBuffer<sFormat>::iterator ite)
{
  int element_index = 0;
  int num_members = sizeof(sFormat) / sizeof(sType);
  sType *member_ptr;
  for (; it < ite; it++)
  {
    member_ptr = (sType *) it;

    if ((sNewType) *(member_ptr) !=0 && (sNewType) *(member_ptr) != 4)

    if (element_index > 99)
      cout << "Element #" << element_index << ":     ";
    else if (element_index > 9)
      cout << "Element # " << element_index << ":     ";
    else
      cout << "Element #  " << element_index << ":     ";

    for (int i = 0; i < num_members; i++) {
      if ((sNewType) *(member_ptr) > 99)
        cout << i << ": " << (sNewType) *(member_ptr) << "    ";
      else if ((sNewType) *(member_ptr) > 9)
        cout << i << ":  " << (sNewType) *(member_ptr) << "    ";
      else
        cout << i << ":   " << (sNewType) *(member_ptr) << "    ";

      member_ptr++; 
    }
    cout << endl;
    element_index++;
  }
 }

#endif /* HEADER_HELPERS */
