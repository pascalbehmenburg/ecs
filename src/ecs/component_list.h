#ifndef COMPONENT_LIST_H_
#define COMPONENT_LIST_H_

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <array>
#include <exception>
#include <map>

#include "type_def.h"

namespace ecs {
/// @file
/// @brief The IComponentList is used as a common interface for all
/// component lists.
/// @details Since the component lists would differ for each
/// different component type this enables us to store all component lists in a
/// single container inside the component manager.
class IComponentList {
 public:
  virtual ~IComponentList() = default;
  virtual void EntityWasDestroyed(Entity entity) = 0;
};

/// @brief The ComponentList stores components of a type T in continuous memory.
/// @details It is a self-managed array which ensures that the memory it
/// allocates stays continuous so that the cache misses when accessing it are
/// minimized. The component manager uses them to keep track of component to
/// entity relations.
template <typename T>
class ComponentList final : public IComponentList {
 public:
  /// @brief Stores a component T associated to the given entity.
  /// @details It stores the component in the next free slot in the array and
  /// keeps track of the entity to component index mapping.
  /// @attention Throws an exception if the entity is already in the list.
  /// @param entity The entity to which the component should be added.
  void Insert(const Entity entity, const T component) {
    if (entity_to_component_index_.contains(entity)) {
      spdlog::dump_backtrace();
      throw std::runtime_error(
          fmt::format("Tried to insert duplicate entity {} in {} list", entity, typeid(T).name()));
    }

    components_[next_component_index_] = component;
    entity_to_component_index_[entity] = next_component_index_;
    component_index_to_entity_[next_component_index_] = entity;
    ++next_component_index_;
    spdlog::debug("Component list inserted component {} for entity {}.", typeid(T).name(), entity);
  }

  /// @brief Removes a component T associated to an entity so that the entity
  /// isn't processed by the systems depending on those components.
  /// @details It overrides the data of the component to be removed with the
  /// data of the last component in the array to ensure continuous memory.
  /// @attention Throws an exception if the entity is not in the list.
  /// @param remove_entity The entity from which the component should be
  /// removed.
  void Remove(const Entity remove_entity) {
    if (!entity_to_component_index_.contains(remove_entity)) {
      throw std::runtime_error(fmt::format("Tried to remove entity {} which is not in {} list",
                                           remove_entity, typeid(T).name()));
    }

    // override all data of the entity which is to be removed with the data of
    // the last entity in the list to ensure continuous memory
    const auto end_component_idx = next_component_index_ - 1;
    const auto remove_component_idx = entity_to_component_index_[remove_entity];

    components_[remove_component_idx] = components_[end_component_idx];

    Entity end_entity = component_index_to_entity_[end_component_idx];
    entity_to_component_index_[end_entity] = remove_component_idx;
    component_index_to_entity_[remove_component_idx] = end_entity;

    // remove the last entity from the list since it is now stored where
    // remove_entity was before
    entity_to_component_index_.erase(remove_entity);
    component_index_to_entity_.erase(end_component_idx);
    --next_component_index_;
  }

  /// @brief Get a component T associated to the given entity.
  /// @details Retrieves T's component list to lookup the entity in it.
  /// @attention Throws an exception if the entity is not in the list.
  /// @param entity The entity for which the component should be returned.
  /// @returns `T&` The component of type T which is associated to the entity.
  T& Get(const Entity entity) {
    if (!entity_to_component_index_.contains(entity)) {
      spdlog::dump_backtrace();
      throw std::runtime_error(
          fmt::format("Tried to get entity {} which is not in {} list", entity, typeid(T).name()));
    }

    spdlog::debug("Component list retrieves component {} for entity {}.", typeid(T).name(), entity);
    return components_[entity_to_component_index_[entity]];
  }

  /// @brief EntityWasDestroyed is called when an entity is destroyed to remove
  /// the component associated to the entity from the component list.
  /// @param entity The entity which was destroyed.
  void EntityWasDestroyed(const Entity entity) override {
    // This fails gracefully if there is no entity in the list enabling us
    // to call this function on all component lists without worrying about
    // whether the component list contains the entity
    if (entity_to_component_index_.contains(entity)) {
      Remove(entity);
      spdlog::debug("Component list removed entity {}.", entity);
    }
  }

 private:
  /// The next free index in the array.
  size_t next_component_index_{};

  /// The array which stores the components.
  std::array<T, kMaxEntities> components_;

  /// Maps an entity to the index of the component in the array.
  std::map<Entity, size_t> entity_to_component_index_;

  /// Maps the index of the component in the array to an entity.
  std::map<size_t, Entity> component_index_to_entity_;
};

}  // namespace ecs

#endif  // COMPONENT_LIST_H_