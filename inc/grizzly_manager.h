#ifndef HEADER_GRIZZLYLIB
#define HEADER_GRIZZLYLIB

#include "grizzly_types.h"

namespace GrizzyLib {
  class FrameGroup {
  private:
      /* buffers[ChannelId][ObjectType] = ObjectId */
      PendingBufferMap buffers;

  public:
      friend class ObjectManager;

      ObjectId getChannelObjectId(ChannelId ch_id, ObjectType obj_type)
      {
          auto iter = buffers.find(ch_id);
          if (iter == buffers.end())
              return 0;
          auto iter2 = iter->second.find(obj_type);
          if (iter2 == iter->second.end())
              return 0;
          return iter2->second;
      };

      ObjectId getChannelObjectId(ChannelId ch_id, ObjectType obj_type)
      {
          return getChannelObjectId(ch_id, obj_type);
      }

      inline bool hasChannelObject(ChannelId ch_id)
      {
          return !( buffers.find(ch_id) == buffers.end() );
      }

      inline bool hasChannelObject(ChannelId ch_id, ObjectType obj_type)
      {
          if (!hasChannelObject(ch_id))
              return false;
          return !( buffers[ch_id].find(obj_type) == buffers[ch_id].end() );
      }

      void addChannelObject(ChannelId ch_id, ObjectType obj_type, ObjectId obj_id)
      {
          if (hasChannelObject(ch_id, obj_type))
              return;
          buffers[ch_id][obj_type] = obj_id;
      }

      template<class sFormat>
      void addChannelObject(ChannelId ch_id, ObjectId obj_id)
      {
          addChannelObject(ch_id, BufferType<sFormat>(), obj_id);
      }
  };

  class ObjectManager
  {
  // private:  // TODO: CHANGE BACK TO PRIVATE!
  public:
      ObjectPtrMap objects;
      uint32_t num_objects;
      uint32_t num_objects_deleted;

      PendingBufferMap pending_buffers;

      uint32_t allocateObject(ObjectType obj_type, UncastPtr block_ptr, UncastPtr obj_ptr)
      {
          num_objects++;
          objects.emplace(num_objects, ObjectPtr{ num_objects, \
                                                  obj_type, \
                                                  block_ptr, \
                                                  obj_ptr });
          
          return num_objects;
      }

      UncastPtr getObject(ObjectId obj_id, ObjectType obj_type)
      {
          auto iter = objects.find(obj_id);
          if (iter == objects.end())
              return 0;
          if (obj_type != iter->second.type)
              return 0;
          return iter->second.obj_ptr;
      }

      void releaseObject(ObjectId obj_id)
      {
          auto obj = objects.find(obj_id);
          if (obj == objects.end())
              return;
          if (obj->second.mem_ptr != 0)
              obj->second.mem_ptr.reset();
          if (obj->second.obj_ptr != 0)
              obj->second.obj_ptr.reset();
          objects.erase(obj);
      }

  public:
      ObjectId allocateFrame()
      {
          auto obj_ptr = std::make_shared<FrameGroup>();
          return allocateObject("frame", 0, obj_ptr);
      }

      FramePtr getFrame(ObjectId obj_id)
      {
          auto obj_ptr = getObject(obj_id, "frame");
          return std::static_pointer_cast< FrameGroup >(obj_ptr);
      }

      void releaseFrame(ObjectId obj_id)
      {
          auto obj_ptr = std::static_pointer_cast<FrameGroup>(getObject(obj_id, "frame"));
          if (obj_ptr == 0)
              return;
          for ( auto it = obj_ptr->buffers.begin(); it != obj_ptr->buffers.end(); ++it )
          {
              for ( auto ite = it->second.begin(); ite != it->second.end(); ++ite )
              {
                  releaseObject(ite->second);
              }
          }
          releaseObject(obj_id);
      }

      void pushPendingBuffer(ChannelId ch_id, ObjectId obj_id)
      {
          pending_buffers[ch_id] = obj_id;
      }

      /*
      template<class sSrcFormat, class sDestFormat>
      ObjectId createConverter()
      {
          auto obj_ptr = std::make_shared< ElementConverter<sSrcFormat, sDestFormat> >();
          return allocateObject(ElementConverter<sSrcFormat, sDestFormat>::converterType(), \
                                                      0, obj_ptr);
      }

      template<class sSrcFormat, class sDestFormat>
      ConverterPtr<sSrcFormat, sDestFormat> getConverter(ObjectId obj_id)
      {
          auto obj_ptr = getObject(obj_id, ElementConverter<sSrcFormat, sDestFormat>::converterType());
          return std::static_pointer_cast< ElementConverter<sSrcFormat, sDestFormat> >(obj_ptr);
      }*/
  };
}

#endif /* HEADER_GRIZZLYLIB */
