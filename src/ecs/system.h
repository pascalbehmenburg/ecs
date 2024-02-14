#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <set>

#include "type_def.h"

namespace ecs {
/// @file
/// @brief A system implements logic to process entities which have a specific
/// set of components that is compatible with the systems requirements.
class System {
 public:
  /// A set of entities which contain the components required by this system.
  std::set<Entity> entities_;

  /// @brief Update is called every frame by the system manager to process all
  /// entities which have components associated to them that are required by
  /// this system.
  virtual void Update() {}
};
}  // namespace ecs

#endif  // SYSTEM_H_
