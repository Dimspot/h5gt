/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5GT_H5GROUP_HPP
#define H5GT_H5GROUP_HPP

#include "H5Object.hpp"
#include "bits/H5_definitions.hpp"
#include "bits/H5Annotate_traits.hpp"
#include "bits/H5Node_traits.hpp"

namespace h5gt {

///
/// \brief Represents an hdf5 group
class Group : public Object,
    public NodeTraits<Group>,
    public AnnotateTraits<Group> {
public:

  // this makes available to use both
  // Object::getObjectType and NodeTraits<T>::getObjectType
  using Object::getObjectType;
  using NodeTraits<Group>::getObjectType;

  const static ObjectType type = ObjectType::Group;

  LinkInfo getLinkInfo() const {
    return Object::getLinkInfo();
  }

  ///
  /// \brief getTargetPath For soft link that returns path to target that
  /// link points to. Otherwise it works the same way as `getPath()`
  /// \param accessProp
  /// \return
  ///
  std::string getTargetPath(
      const LinkAccessProps& accessProp = LinkAccessProps()) const{
    if (getLinkInfo().getLinkType() == LinkType::Soft){
      char str[256];

      if (H5Lget_val(getId(false), getPath().c_str(),
                     &str, 255, accessProp.getId(false)) < 0){
        HDF5ErrMapper::ToException<GroupException>(
              std::string("Can't get path to which the link points to"));
      }
      return std::string{str};
    }

    return getPath();
  }

  /// \brief operator == Check if objects reside in the same file and equal to each other
  /// \param other
  /// \return
  bool operator==(const Group& other) const {
    ObjectInfo leftOInfo = getObjectInfo();
    ObjectInfo rightOInfo = other.getObjectInfo();

    if (leftOInfo.getFileNumber() != rightOInfo.getFileNumber() ||
        leftOInfo.getFileNumber() == 0 ||
        rightOInfo.getFileNumber() == 0)
      return false;

  #if (H5Oget_info_vers < 3)
    return getAddress() == other.getAddress();
  #else
    int tokenCMP;
    H5O_token_t leftToken = leftOInfo.getHardLinkToken();
    H5O_token_t rightToken = rightOInfo.getHardLinkToken();

    if (H5Otoken_cmp(getFileId(false), &leftToken, &rightToken, &tokenCMP) < 0){
      HDF5ErrMapper::ToException<DataSetException>(
            "Unable compare tokens");
    }

    return !tokenCMP;
  #endif
  }

  bool operator!=(const Group& other) const {
    return !(*this == other);
  }

  static Group FromId(const hid_t& id, const bool& increaseRefCount = false){
    Object obj = Object(id, ObjectType::Group, increaseRefCount);
    return Group(obj);
  };

protected:
  Group(const Object& obj) : Object(obj){};
  using Object::Object;

  inline Group(Object&& o) noexcept : Object(std::move(o)) {};

  friend class File;
  friend class Reference;
  template <typename Derivate> friend class ::h5gt::NodeTraits;
};

}  // namespace h5gt

#endif // H5GT_H5GROUP_HPP
