


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

    //if (element_index > 99)
      // cout << "Element #" << element_index << ":     ";
    //else if (element_index > 9)
      // cout << "Element # " << element_index << ":     ";
    //else
      // cout << "Element #  " << element_index << ":     ";

    //for (int i = 0; i < num_members; i++) {
    //  if ((sNewType) *(member_ptr) > 99)
        // cout << i << ": " << (sNewType) *(member_ptr) << "    ";
    //  else if ((sNewType) *(member_ptr) > 9)
        // cout << i << ":  " << (sNewType) *(member_ptr) << "    ";
    //  else
        // cout << i << ":   " << (sNewType) *(member_ptr) << "    ";

      member_ptr++; 
    }
    // cout << endl;
    element_index++;
  }
 }
