#ifndef HEADER_OBJ_FACTORY
#define HEADER_OBJ_FACTORY

class FrameGroup {
private:
  /* buffers[ChannelId][ObjectType] = ObjectId */
  std::unordered_map<ChannelId , std::unordered_map<std::string, ObjectId> > buffers;

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

  template<class sFormat>
  ObjectId getChannelObjectId(ChannelId ch_id)
  {
    return getChannelObjectId(ch_id, BufferType<sFormat>());
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

typedef std::shared_ptr<FrameGroup> FramePtr;

class ObjectManager {
// private:  // TODO: CHANGE BACK TO PRIVATE!
public:
    struct ObjectPtr {
        uint32_t id;
        ObjectType type;
        std::shared_ptr<void> mem_ptr;
        std::shared_ptr<void> obj_ptr;
    };
    std::unordered_map<ObjectId, ObjectPtr> objects;
    uint32_t num_objects;
    uint32_t num_objects_deleted;

    std::unordered_map<ChannelId, ObjectId> pending_buffers;

    uint32_t allocateObject(ObjectType obj_type, std::shared_ptr<void> block_ptr, std::shared_ptr<void> obj_ptr)
    {
      num_objects++;
      objects.emplace(num_objects, ObjectPtr{ num_objects, \
                                              obj_type, \
                                              block_ptr, \
                                              obj_ptr });
      // cout << " === Object #" << num_objects << " allocated === " << endl;
      return num_objects;
    }

    std::shared_ptr<void> getObject(ObjectId obj_id, ObjectType obj_type)
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
      // cout << " === Object #" << num_objects_deleted++ << " released === " << endl;
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

    template<class sFormat>
    ObjectId allocateBuffer(uint32_t size)
    {
      auto block_ptr = std::make_shared< std::vector<sFormat> >(size);
      auto obj_ptr = std::make_shared< ElementBuffer<sFormat> >(&*block_ptr->begin(), &*block_ptr->end());
      return allocateObject(ElementBuffer<sFormat>::bufferType(), block_ptr, obj_ptr);
    }

    void releaseBuffer(ObjectId obj_id)
    {
      releaseObject(obj_id);
    }

    template<class sFormat>
    BufferPtr<sFormat> getBuffer(ObjectId obj_id)
    {
      auto obj_ptr = getObject(obj_id, ElementBuffer<sFormat>::bufferType());
      return std::static_pointer_cast< ElementBuffer<sFormat> >(obj_ptr);
    }

    void pushPendingBuffer(ChannelId ch_id, ObjectId obj_id)
    {
      pending_buffers[ch_id] = obj_id;
    }

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
    }

};

#endif /* HEADER_OBJ_FACTORY */
